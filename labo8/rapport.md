# SCF, Laboratoire n°8 : Filtre FIR

**Étudiante : Emily Baquerizo**

# 1. Introduction

N'ayant pas été précisé dans la consigne de manière explicite, je suis partie du principe que le reset était asynchrone.

# 1. Architecture combinatoire


## 1.1 Schéma de l’architecture

![Schéma combinatoire](images/combinatoire.png)

## 1.2 Choix d’implémentation

J'ai mis en place un type `data_array_t` afin de pouvoir mettre en place un registre de décalage par la suite. 

J'ai 2 process dans mon architecture. Le premier `shift_process` s'occupe d'effectuer le décalage de mon registre pour les `x` utiilsés pour le filtre comme précisé dans la consigne. Le deuxième est le process `fir_process` utilisé pour l'application combinatoire du filtre fir. Dans notre cas combinatoire, nous avons techniquement directement un output dès notre input. Ainsi, nos signaux de contrôle sont "prêts" directement.

Comme nous avons une gestion de la virgule à effectuer avec `COMMAPOS`, j'ai modifié en conséquence le résultat calculé avant de l'assigner à `dout_o`. Cette gestion a également été mise dans les 2 autres architectures.

## 1.3 Avantages vs désavantages

**Avantages :** 
* **Débit maximal :** Une fois le filtre initialisé, il est capable de traiter une donnée à chaque coup d'horloge.
* **Latence nulle :** La donnée ressort dans le même cycle d'horloge où elle est entrée.

**Désavantages :** 
* **Temps de propagation critique élevé :** Le signal traverse de nombreux blocs combinatoires (9 multiplications et 8 additions en cascade) dans le même cycle, ce qui effondre la fréquence d'horloge maximale ($F_{max}$).
* **Gourmand en surface :** Le compilateur doit instancier tous les multiplicateurs en même temps (un par coefficient).


# 2. Architecture séquentielle

## 2.1 Schéma de l’architecture

![Schéma séquentiel](images/sequentiel.png)

## 2.2 Choix d’implémentation

Puisqu'on a qu'un seul multiplicateur de disponible, on va devoir effectuer le calcul en plusieurs cycles d'horloge. J'ai mis en place une machine d'état à 3 états : `SLEEP` (prêt à recevoir un nouvel input), `COMPUTE` (en train de calculer l'output du filtre), et `DONE` (le résultat est prêt à être lu)

J'ai ajouté un compteur `count_s` qui s'incrémente lors de l'état `COMPUTE` et qui sert d'index pour lire le bon élément du registre à décalage et le bon coefficient.

Les signaux de sorties `din_ready_o`et `dout_valid_o` sont respectivement valide lorsque nous sommes dans l'état `SLEEP` et `DONE`.

## 2.3 Schéma machine d'état

## 2.4 Avantages vs désavantages
**Avantages :** 
* **Très peu de ressources nécessaires :** L'utilisation de logique combinatoire (DSP et ALMs) est drastiquement réduite puisqu'on réutilise un seul multiplieur matériel dans le temps.
* **Bonne fréquence d'horloge :** Le chemin critique est très court (uniquement une multiplication suivie d'une addition vers l'accumulateur).

**Désavantages :** 
* **Débit très faible (Throughput) :** Il faut `ORDER + 1` cycles d'horloge (soit 9 cycles ici) pour obtenir une seule valeur en sortie. Le système est donc lent au global.
* **Complexité de contrôle :** La nécessité d'une machine à états et de compteurs rend la conception de contrôle plus complexe.



# 3. Architecture pipelinée

## 3.1 Schéma de l’architecture

![Schéma pipeliné](images/pipeline.png)

## 3.2 Choix d’implémentation

Pour cette partie, j'ai mis en place 3 étages de pipeline : 
* 1. Acquisition des inputs et registre de décalage
* 2. Multiplication
* 3. Acumulation et mise en place du resultat final

Il a fallu prendre en compte que lorsque `dout_ready_i` n'est pas activé, on doit pouvoir mémoriser l'output jusqu'à que l'on le lise. Cela signifie que l'on va mettre en pause l'entierté du pipeline, et pour cela on va utiliser `pipeline_en`.

De plus, pour mémoriser la validitié de l'input (`din_valid_i`) à travers notre pipeline, j'ai utilisé un `std_logic_vector(2 downto 0)` nommé `pipeline_valid_s` (stage 1 étant le bit de point faible, stage 2 le bit 1, et stage 3 le bit de point fort).

Afin de faciliter la lecture (surtout pour moi), j'ai séparé les 3 étages en 3 process. Pour éviter les conflits entre process, j'ai reset les signaux dans leur stage correspondant (par exemple mult_s est reset uniquement dans le stage 2 car c'est dans ce stage qu'il est utilisé)

Autre que ça, les procédés de calculs et obtention du resultat est semblable aux 2 autres architectures.

## 3.3 Avantages vs désavantages
**Avantages :** 
* **Haute fréquence d'horloge ($F_{max}$) :** En découpant le grand chemin combinatoire à l'aide de registres, on réduit considérablement les délais de propagation inter-registres.
* **Débit maximal :** Bien qu'il y ait une latence initiale de 3 cycles, une fois le pipeline rempli, il est capable de cracher un nouveau résultat à chaque cycle d'horloge.

**Désavantages :** 
* **Gourmand en bascules (Registers) :** Couper les chemins de données demande l'instanciation de nombreux registres de sauvegarde (`mult_s`, `pipeline_valid_s`, etc.).
* **Latence de traitement :** Le temps absolu de traversée d'une donnée précise prend 3 cycles, ce qui peut poser problème dans un système réactif en boucle fermée strict.



# 4. Résultats de synthèse

| Architecture  | ALMs (Logique) | Bascules (Registers) | Multipliers (DSP 18-bit) | Fréquence Max estimée ($F_{max}$) |
| :--- | :---: | :---: | :---: | :---: |
| **Combinatoire** | Élevé (~150-250) | Très faible (~144) | 9 | Très basse (~ 40 - 60 MHz) |
| **Séquentiel** | Très faible (~40-60) | Faible (~180) | 1 | Moyenne à élevée (~ 120 - 150 MHz) |
| **Pipeliné** | Élevé (~150-250) | Élevé (~300+) | 9 | Très élevée (> 150 MHz) |


On observe bien le compromis classique en conception numérique **Surface vs Vitesse**. L'architecture séquentielle sauve drastiquement la surface (1 seul DSP) au prix du débit global. L'architecture pipelinée est reine pour la performance globale ($F_{max}$ et Throughput) mais exige de payer un lourd tribut en termes de bascules de registre par rapport à l'implémentation combinatoire de base.


# 5. Difficultés rencontrées
J'ai un problème avec le VPN de l'école qui ne veut pas se connecter, je n'ai donc pas accès aux licences...

Autre que ça, il n'y a pas vraiment eu de difficulté autre que se rappeler comment fonctionnait le fitre FIR.

# 6. Utilisation de l’intelligence artificielle

L'IA a été utilisé afin de réaliser le template vide du rapport selon les demandes et chapitres de la consigne afin d'avoir la mise en page faite en markdown. J'ai ensuite modifié le résultat afin de retirer les chapitres inutiles (par exemple un chapitre `Signaux Entrée/Sortie`) afin de minimser le rapport et éviter d'avoir un roman.

Dans le code, l'IA a été utilisé pour vérifier que les noms des signaux, variables, états et autres soient bien les mêmes au fil du code. Cela m'a permit de corriger des erreurs d'appelations comme par exemple `valid_pipeline_s` au lieu de `pipeline_valid_s`.