--==============================================================================
--  File        : fir_filter.vhd : Combinational
--  Description : Finite Impulse Response (FIR) Filter
--
--  Author      : REDS institute
--  Date        : 2026
--==============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.fir_filter_pkg.all;

entity fir_filter is
  generic (
    ORDER       : positive := 8;    -- Filter order
    DATASIZE    : positive := 16;   -- Input/output data width
    COEFFSIZE   : positive := 16;   -- Coefficient width
    COMMAPOS    : natural := 0      -- Position of comma for computations
  );
  port (
    clk_i        : in  std_logic;
    rst_i        : in  std_logic;
    din_valid_i  : in  std_logic;
    din_i        : in  std_logic_vector(DATASIZE-1 downto 0);
    din_ready_o  : out std_logic;
    coeffs_i     : in  coeff_array(0 to ORDER);
    dout_valid_o : out std_logic;
    dout_o       : out std_logic_vector(DATASIZE-1 downto 0);
    dout_ready_i : in  std_logic
  );
end entity fir_filter;

architecture combinational of fir_filter is 
--- Internal signals and types can be declared here if needed

  type data_array_t is array (0 to ORDER) of signed(DATASIZE - 1 downto 0); -- Type for shit register
  signal shift_reg_s : data_array_t;

begin

--- Process for shift register
  shift_process : process(clk_i, rst_i)
  begin
    if rst_i = '1' then
      shift_reg_s <= (others => (others => '0'));
    elsif rising_edge(clk_i) then
      if din_valid_i = '1' and dout_ready_i = '1' then
        shift_reg_s(0) <= signed(din_i);

        -- Shift
        for i in 1 to ORDER loop
          shift_reg_s(i) <= shift_reg_s(i - 1);
        end loop;

      end if;
    end if;
  end process;

--- FIR filter
  fir_process : process(din_i, shift_reg_s, coeffs_i)
    variable mult_v : signed(DATASIZE + COEFFSIZE - 1 downto 0);
    --- Ajout de 8 bit pour être sûre de pas avoir un overflow lors du calcul final
    variable result_v : signed(DATASIZE + COEFFSIZE + 7 downto 0);
  begin
    result_v := (others => '0');

    -- First init needed
    mult_v := signed(din_i) * coeffs_i(0);
    result_v := result_v + resize(mult_v, result_v'length);

    --- y[n] = b0x[n] + b1x[n − 1] + b2x[n − 2] + · · · + bN x[n − N]
    for i in 1 to ORDER loop
      mult_v := shift_reg_s(i - 1) * coeffs_i(i);
      result_v := result_v + resize(mult_v, result_v'length);
    end loop;

    --- Output with COMMAPOS
    dout_o <= std_logic_vector(result_v(DATASIZE + COMMAPOS - 1 downto COMMAPOS));

  end process;

  -- Combiantional : 
  din_ready_o <= dout_ready_i;
  dout_valid_o <= din_valid_i;

end architecture combinational;