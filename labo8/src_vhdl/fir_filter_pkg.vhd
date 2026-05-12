--==============================================================================
--  File        : fir_filter_pkg.vhd
--  Description : FIR Filter Package - Common types and constants
--
--  Author      : REDS institute
--  Date        : 2026
--==============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package fir_filter_pkg is

  -- Coefficient array type (parameterized by NBTAPS and COEFF_SIZE)
  -- Note: Must be instantiated in architecture with appropriate constraints
  type coeff_array is array (natural range <>) of signed(15 downto 0);

end package fir_filter_pkg;

package body fir_filter_pkg is
end package body fir_filter_pkg;
