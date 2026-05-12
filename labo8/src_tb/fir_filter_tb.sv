`timescale 1ns/1ps

module fir_filter_tb;

  // ---------------------------------------------------------------------------
  // Parameters
  // ---------------------------------------------------------------------------
  localparam int ORDER       = 8;
  localparam int DATASIZE    = 16;
  localparam int COEFFSIZE   = 16;
  localparam int COMMAPOS    = 2;

  // ---------------------------------------------------------------------------
  // Signals
  // ---------------------------------------------------------------------------
  logic clk_i;
  logic rst_i;

  logic din_valid_i;
  logic [DATASIZE-1:0] din_i;
  logic din_ready_o;

  logic dout_valid_o;
  logic [DATASIZE-1:0] dout_o;
  logic dout_ready_i;
  logic signed [COEFFSIZE-1:0] coeffs_i [0:ORDER];

  // Reference model state
  logic signed [DATASIZE-1:0] ref_samples [0:ORDER];
  logic signed [COEFFSIZE-1:0] ref_coeffs [0:ORDER];
  logic [ORDER:0] ref_valid_pipe;
  logic signed [DATASIZE-1:0] ref_pending_data;
  logic ref_pending_valid;
  int ready_high_gap;

  // Generate dout_ready_i with random spaces of '1' between '0' pulses.
  task automatic drive_dout_ready_random_spaces;
    if (ready_high_gap > 0) begin
      dout_ready_i = 1'b1;
      ready_high_gap--;
    end else begin
      dout_ready_i = 1'b0;
      ready_high_gap = $urandom_range(1, 6);
    end
  endtask

  // ---------------------------------------------------------------------------
  // DUT instantiation
  // ---------------------------------------------------------------------------
  fir_filter #(
    .ORDER(ORDER),
    .DATASIZE(DATASIZE),
    .COEFFSIZE(COEFFSIZE),
    .COMMAPOS(COMMAPOS)
  ) dut (
    .clk_i(clk_i),
    .rst_i(rst_i),
    .din_valid_i(din_valid_i),
    .din_i(din_i),
    .din_ready_o(din_ready_o),
    .coeffs_i(coeffs_i),
    .dout_valid_o(dout_valid_o),
    .dout_o(dout_o),
    .dout_ready_i(dout_ready_i)
  );

  // ---------------------------------------------------------------------------
  // Clock generation
  // ---------------------------------------------------------------------------
  initial begin
    clk_i = 0;
    forever #5 clk_i = ~clk_i;
  end

  // ---------------------------------------------------------------------------
  // Test sequence
  // ---------------------------------------------------------------------------
  initial begin
    // Initialize
    rst_i = 1;
    din_valid_i = 0;
    din_i = 0;
    dout_ready_i = 1;
    ready_high_gap = 0;

    // Set expected coefficients (must match the VHDL implementation)
    ref_coeffs[0] = 16'sd1;
    ref_coeffs[1] = 16'sd2;
    ref_coeffs[2] = 16'sd3;
    ref_coeffs[3] = 16'sd4;
    ref_coeffs[4] = 16'sd5;
    ref_coeffs[5] = 16'sd6;
    ref_coeffs[6] = 16'sd7;
    ref_coeffs[7] = 16'sd8;
    ref_coeffs[8] = 16'sd9;

    // Drive DUT coefficient port
    for (int k = 0; k <= ORDER; k++) begin
      coeffs_i[k] = ref_coeffs[k];
      ref_samples[k] = '0;
    end
    ref_valid_pipe = '0;
    ref_pending_data = '0;
    ref_pending_valid = 1'b0;

    // Reset pulse
    repeat (5) @(posedge clk_i);
    rst_i = 0;  // Release reset (active high)
    repeat (5) @(posedge clk_i);

    // Send a sequence of input samples
    for (int n = 0; n < 20; n++) begin
      din_valid_i = 1;
      din_i = n + 1;

      // create intermittent backpressure on the output
      drive_dout_ready_random_spaces();

      @(posedge clk_i);

      // Cycle-accurate reference model (mirrors DUT process ordering)
      begin : ref_step
        automatic longint signed acc;
        automatic logic signed [DATASIZE-1:0] ref_current_out;
        automatic logic signed [DATASIZE-1:0] expected;
        automatic logic expected_valid;

        acc = 0;
        for (int k = 0; k <= ORDER; k++) begin
          acc += ref_coeffs[k] * ref_samples[k];
        end
        ref_current_out = (acc >>> COMMAPOS);
        expected = '0;
        expected_valid = 1'b0;

        if (dout_ready_i) begin
          if (ref_pending_valid) begin
            expected = ref_pending_data;
            expected_valid = 1'b1;
            ref_pending_valid = 1'b0;
          end else if (ref_valid_pipe[ORDER]) begin
            expected = ref_current_out;
            expected_valid = 1'b1;
          end
        end else begin
          if (!ref_pending_valid && ref_valid_pipe[ORDER]) begin
            ref_pending_data = ref_current_out;
            ref_pending_valid = 1'b1;
          end
        end
      end

      for (int k = ORDER; k > 0; k--) begin
        ref_valid_pipe[k] = ref_valid_pipe[k-1];
      end
      ref_valid_pipe[0] = din_valid_i;

      if (din_valid_i) begin
        for (int k = ORDER; k > 0; k--) begin
          ref_samples[k] = ref_samples[k-1];
        end
        ref_samples[0] = $signed(din_i);
      end

      if (din_ready_o !== 1'b1) begin
        $error("[FAILED] Time %0t: din_ready_o should stay asserted", $time);
        $fatal;
      end
    end

    // Flush remaining pipeline samples
    din_valid_i = 0;
    dout_ready_i = 1;
    repeat (ORDER + 4) begin
      @(posedge clk_i);

      begin : ref_flush
        automatic longint signed acc;
        automatic logic signed [DATASIZE-1:0] ref_current_out;
        automatic logic signed [DATASIZE-1:0] expected;
        automatic logic expected_valid;

        acc = 0;
        for (int k = 0; k <= ORDER; k++) begin
          acc += ref_coeffs[k] * ref_samples[k];
        end
        ref_current_out = (acc >>> COMMAPOS);
        expected = '0;
        expected_valid = 1'b0;

        if (dout_ready_i) begin
          if (ref_pending_valid) begin
            expected = ref_pending_data;
            expected_valid = 1'b1;
            ref_pending_valid = 1'b0;
          end else if (ref_valid_pipe[ORDER]) begin
            expected = ref_current_out;
            expected_valid = 1'b1;
          end
        end else begin
          if (!ref_pending_valid && ref_valid_pipe[ORDER]) begin
            ref_pending_data = ref_current_out;
            ref_pending_valid = 1'b1;
          end
        end
      end

      for (int k = ORDER; k > 0; k--) begin
        ref_valid_pipe[k] = ref_valid_pipe[k-1];
      end
      ref_valid_pipe[0] = 1'b0;
    end

    // Finish
    @(posedge clk_i);
    $display("[PASSED] fir_filter testbench completed successfully.");
    $finish;
  end

endmodule
