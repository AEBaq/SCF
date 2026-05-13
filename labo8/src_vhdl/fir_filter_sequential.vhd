--==============================================================================
--  File        : fir_filter.vhd : Sequential
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

architecture sequential of fir_filter is 
    type state_t is (SLEEP, COMPUTE, DONE);
    signal state_s : state_t;

    -- Signal to hold the intermediate result of the FIR computation, with enough bits to avoid overflow
    signal result_s : signed(DATASIZE + COEFFSIZE + 7 downto 0);
    -- Counter to keep track of the number of processed samples
    signal count_s : natural range 0 to ORDER;
    
    type data_array_t is array (0 to ORDER) of signed(DATASIZE - 1 downto 0); -- Type for shift register
    signal shift_reg_s : data_array_t;

begin

    fir_process : process(clk_i, rst_i)
        variable next_state_s : state_t;
    begin
        if rst_i = '1' then
            state_s <= SLEEP;
            count_s <= 0;
            result_s <= (others => '0');
            shift_reg_s <= (others => (others => '0'));
        elsif rising_edge(clk_i) then
            next_state_s := state_s; -- Default to hold state
            case state_s is
            when SLEEP =>
                if din_valid_i = '1' then
                    shift_reg_s(0) <= signed(din_i);

                    for i in 1 to ORDER loop
                        shift_reg_s(i) <= shift_reg_s(i - 1);
                    end loop;

                    next_state_s := COMPUTE;
                end if;

            when COMPUTE =>
                if count_s = 0 then
                    result_s <= resize(shift_reg_s(0) * coeffs_i(0), result_s'length);
                else
                    result_s <= result_s + resize(shift_reg_s(count_s) * coeffs_i(count_s), result_s'length);
                end if;
                if count_s = ORDER then
                    next_state_s := DONE;
                else 
                    count_s <= count_s + 1;
                end if;
            when DONE =>
                if dout_ready_i = '1' then
                    next_state_s := SLEEP;
                    count_s <= 0;
                end if;
            end case;
            state_s <= next_state_s;
        end if;
    end process;

    dout_o <= std_logic_vector(result_s(DATASIZE + COMMAPOS - 1 downto COMMAPOS));

    -- Control signals
    din_ready_o <= '1' when state_s = SLEEP else '0';
    dout_valid_o <= '1' when state_s = DONE else '0';


end architecture sequential;