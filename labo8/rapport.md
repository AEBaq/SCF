# SCF, Laboratoire n°8 : Filtre FIR

# 1. Introduction

> Note : J'ai un problème avec le VPN de l'école qui ne veut pas se connecter, je n'ai donc pas accès au licence...

## 1.1 Objectif du laboratoire

Le but de ce laboratoire est de concevoir et implémenter un filtre FIR (Finite Impulse Response) selon trois architectures différentes :

- Architecture combinatoire
- Architecture séquentielle
- Architecture pipelinée

Le laboratoire permet d’étudier les compromis entre :
- ressources matérielles utilisées,
- fréquence maximale atteignable,
- latence,
- débit de traitement.


## 1.2 Rappel théorique

Un filtre FIR d’ordre `N` calcule sa sortie selon l’équation :

:contentReference[oaicite:0]{index=0}

avec :
- `x[n]` : signal d’entrée,
- `y[n]` : signal de sortie,
- `b_i` : coefficients du filtre.

Le filtre utilise un registre à décalage permettant de conserver les `N+1` dernières valeurs d’entrée.


# 2. Cahier des charges

## 2.1 Paramètres génériques

| Paramètre | Description |
|---|---|
| `ORDER` | Ordre du filtre |
| `DATASIZE` | Taille des données |
| `COEFFSIZE` | Taille des coefficients |
| `COMMAPOS` | Position de la virgule fixe |


## 2.2 Interface du module

```vhdl
entity fir_filter is
generic (
    ORDER : positive := 8;
    DATASIZE : positive := 16;
    COEFFSIZE : positive := 16;
    COMMAPOS : natural := 0
);
port (
    clk_i : in std_logic;
    rst_i : in std_logic;
    din_valid_i : in std_logic;
    din_i : in std_logic_vector(DATASIZE-1 downto 0);
    din_ready_o : out std_logic;
    coeffs_i : in coeff_array(0 to ORDER);
    dout_valid_o : out std_logic;
    dout_o : out std_logic_vector(DATASIZE-1 downto 0);
    dout_ready_i : in std_logic
);
end entity fir_filter;
```

# 3. Architecture combinatoire


## 3.1 Schéma de l’architecture

![Schéma combinatoire](images/combinatoire.png)

## 3.2 Choix d’implémentation

Décrire :

* structure de l’arbre d’addition,
* types numériques utilisés,
* gestion du handshake,
* gestion du débordement/troncature.

J'ai mis en place un type `data_array_t` afin de pouvoir mettre en place un registre de décalage par la suite. 

J'ai 2 process dans mon architecture. Le premier `shift_process` s'occupe d'effectuer le décalage de mon registre pour les `x` utiilsés pour le filtre comme précisé dans la consigne. Le deuxième est le process `fir_process` utilisé pour l'application combinatoire du filtre fir. Dans notre cas combinatoire, nous avons techniquement directement un output dès notre input. Ainsi, nos signaux de contrôle sont "prêts" directement.

Comme nous avons une gestion de la virgule à effectuer avec `COMMAPOS`, j'ai modifié en conséquence le résultat calculé avant de l'assigner à `dout_o`.

## 3.3 Avantages vs désavantages



# 4. Architecture séquentielle

## 4.1 Schéma de l’architecture

![Schéma séquentiel](images/sequentiel.png)

## 4.2 Choix d’implémentation

Puisqu'on a qu'un seul multiplicateur de disponible, on va devoir effectuer le calcul en plusieurs cycles d'horloge. J'ai mis en place une machine d'état à 3 états : `SLEEP` (prêt à recevoir un nouvel input), `COMPUTE` (en train de calculer l'output du filtre), et `DONE` (le résultat est prêt à être lu)

Les signaux de sorties `din_ready_o`et `dout_valid_o` sont respectivement valide lorsque nous sommes dans l'état `SLEEP` et `DONE`.

## 4.3 Avantages vs désavantages

# 5. Architecture pipelinée

## 5.1 Schéma de l’architecture

![Schéma pipeliné](images/pipeline.png)

## 5.2 Choix d’implémentation

Décrire :

* nombre d’étages pipeline,
* équilibre des calculs,
* gestion de la validité des données.

## 5.3 Avantages vs désavantages

# 6. Vérification et simulations

## 6.1 Banc de test

Décrire :

* fonctionnement du testbench,
* stimuli utilisés,
* vérification des résultats.

## 6.2 Résultats de simulation

### Architecture combinatoire

> Insérer captures d’écran et commentaires.

![Simulation combinatoire](images/sim_comb.png)


### Architecture séquentielle

![Simulation séquentielle](images/sim_seq.png)


### Architecture pipelinée

![Simulation pipeline](images/sim_pipe.png)


# 7. Résultats de synthèse

## 7.1 Cible utilisée

* Carte : DE1-SoC
* FPGA : (à compléter)
* Outil : Quartus Prime (version)


## 7.2 Tableau récapitulatif

| Architecture | ALUTs / Logic | Registres | DSP | Fmax | Latence |
| ------------ | ------------- | --------- | --- | ---- | ------- |
| Combinatoire |               |           |     |      |         |
| Séquentielle |               |           |     |      |         |
| Pipelinée    |               |           |     |      |         |


## 7.3 Analyse des résultats

Décrire :

* impact du pipeline sur Fmax,
* coût matériel,
* compromis ressources/performance,
* intérêt de chaque architecture selon le contexte.


# 8. Difficultés rencontrées

Décrire :

* problèmes de timing,
* gestion des tailles de mots,
* synchronisation,
* bugs rencontrés,
* solutions apportées.


# 9. Utilisation de l’intelligence artificielle

Décrire précisément :

* aide à la compréhension,
* génération de code,
* aide au debug,
* rédaction du rapport,
* explications théoriques.


L'IA a été utilisé afin de réaliser le template vide du rapport selon les demandes et chapitres de la consigne. J'ai ensuite modifié le résultat afin de retirer les chapitres inutiles (par exemple un chapitre `Signaux Entrée/Sortie`) afin de minimser le rapport et éviter d'écrire un roman.

# 10. Conclusion

Résumer :

* les résultats obtenus,
* les différences entre architectures,
* les enseignements tirés du laboratoire.

