--==============================================================================
--  File        : fir_filter_synth.vhd
--  Description : Synthesis wrapper for fir_filter with fixed coefficients
--
--  Author      : REDS institute
--  Date        : 2026
--==============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.fir_filter_pkg.all;

entity fir_filter_synth is
  generic (
    DATASIZE  : positive := 16;
    COEFFSIZE : positive := 16;
    COMMAPOS  : natural  := 0
  );
  port (
    clk_i        : in  std_logic;
    rst_i        : in  std_logic;
    din_valid_i  : in  std_logic;
    din_i        : in  std_logic_vector(DATASIZE-1 downto 0);
    din_ready_o  : out std_logic;
    dout_valid_o : out std_logic;
    dout_o       : out std_logic_vector(DATASIZE-1 downto 0);
    dout_ready_i : in  std_logic
  );
end entity fir_filter_synth;

architecture rtl of fir_filter_synth is

  constant ORDER_C : positive := 8;

  -- Fixed coefficient values (ORDER_C+1 taps)
  constant COEFFS_INIT_C : coeff_array(0 to ORDER_C) := (
    to_signed(1, COEFFSIZE),
    to_signed(2, COEFFSIZE),
    to_signed(3, COEFFSIZE),
    to_signed(4, COEFFSIZE),
    to_signed(5, COEFFSIZE),
    to_signed(6, COEFFSIZE),
    to_signed(7, COEFFSIZE),
    to_signed(8, COEFFSIZE),
    to_signed(9, COEFFSIZE)
  );

  signal coeffs_reg     : coeff_array(0 to ORDER_C);
  signal fir_dout       : std_logic_vector(DATASIZE-1 downto 0);
  signal fir_dout_valid : std_logic;
  signal dout_reg       : std_logic_vector(DATASIZE-1 downto 0);
  signal dout_valid_reg : std_logic;

begin

  dout_o <= dout_reg;
  dout_valid_o <= dout_valid_reg;

  u_fir_filter : entity work.fir_filter
    generic map (
      ORDER     => ORDER_C,
      DATASIZE  => DATASIZE,
      COEFFSIZE => COEFFSIZE,
      COMMAPOS  => COMMAPOS
    )
    port map (
      clk_i        => clk_i,
      rst_i        => rst_i,
      din_valid_i  => din_valid_i,
      din_i        => din_i,
      din_ready_o  => din_ready_o,
      coeffs_i     => coeffs_reg,
      dout_valid_o => fir_dout_valid,
      dout_o       => fir_dout,
      dout_ready_i => dout_ready_i
    );

  p_coeffs_reg : process(clk_i, rst_i)
  begin
    if rst_i = '1' then
        coeffs_reg <= (others => to_signed(0, 16));
    elsif rising_edge(clk_i) then
      coeffs_reg <= COEFFS_INIT_C;
    end if;
  end process;

  p_output_reg : process(clk_i, rst_i)
  begin
    if rst_i = '1' then
      dout_reg <= (others => '0');
      dout_valid_reg <= '0';
    elsif rising_edge(clk_i) then
      if dout_ready_i = '1' then
        dout_reg <= fir_dout;
        dout_valid_reg <= fir_dout_valid;
      end if;
    end if;
  end process;

end architecture rtl;
