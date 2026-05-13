--==============================================================================
--  File        : fir_filter.vhd : Pipeline
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

architecture pipeline of fir_filter is 

    type data_array_t is array (0 to ORDER) of signed(DATASIZE - 1 downto 0); -- Type for shift register
    signal shift_reg_s : data_array_t;

    type mult_array_t is array (0 to ORDER) of signed(DATASIZE + COEFFSIZE - 1 downto 0); -- Type for multipliers
    signal mult_s : mult_array_t;

    signal result_s : std_logic_vector(DATASIZE - 1 downto 0); -- Signal for the final output result

    signal pipeline_valid_s : std_logic_vector(2 downto 0); -- Shift register to track the validity throufh the pipeline stages

    signal pipeline_en : std_logic; -- Signal to enable the pipeline stages in case dout_ready_i is not ready

begin

    pipeline_en <= dout_ready_i;

    stage_1 : process(clk_i, rst_i)
    begin

        if rst_i = '1' then
            shift_reg_s <= (others => (others => '0'));
            pipeline_valid_s(0) <= '0';
        elsif rising_edge(clk_i) then
            if pipeline_en = '1' then
                -- 1st stage : Input and shift register
                shift_reg_s(0) <= signed(din_i);
                for i in 1 to ORDER loop
                    shift_reg_s(i) <= shift_reg_s(i - 1);
                end loop;

                pipeline_valid_s(0) <= din_valid_i;
            end if;
        end if;
    end process;


    -- 2nd stage : Multiplication
    stage_2 : process(clk_i, rst_i)
    begin
        if rst_i = '1' then
            mult_s <= (others => (others => '0'));
            pipeline_valid_s(1) <= '0';
        elsif rising_edge(clk_i) then
            if pipeline_en = '1' then
                for i in 0 to ORDER loop
                    mult_s(i) <= shift_reg_s(i) * coeffs_i(i);
                end loop;

                pipeline_valid_s(1) <= pipeline_valid_s(0);
            end if;
        end if;
    end process;


    -- 3rd stage : Accumulation
    stage_3 : process(clk_i, rst_i)
        variable sum_v : signed(DATASIZE + COEFFSIZE + 7 downto 0);
    begin
        if rst_i = '1' then
            result_s <= (others => '0');
            pipeline_valid_s(2) <= '0';
        elsif rising_edge(clk_i) then
            if pipeline_en = '1' then
                sum_v := (others => '0');
                for i in 0 to ORDER loop
                    sum_v := sum_v + resize(mult_s(i), sum_v'length);
                end loop;

                pipeline_valid_s(2) <= pipeline_valid_s(1);

                -- Output with COMMAPOS
                result_s <= std_logic_vector(sum_v(DATASIZE + COMMAPOS - 1 downto COMMAPOS));
            end if;
        end if;
    end process;

    din_ready_o <= pipeline_en; -- Ready to accept new input when pipeline is enabled
    dout_valid_o <= pipeline_valid_s(2); -- if the last stage is valid, then output is valid
    dout_o <= result_s;


end architecture pipeline;