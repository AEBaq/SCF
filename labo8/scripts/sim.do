#!/usr/bin/tclsh

# FIR Filter Simulation Script
# Supports: fir_filter_tb

#------------------------------------------------------------------------------
proc vhdl_compil { } {
  global Path_VHDL
  global Path_TB
  puts "\n=== VHDL Compilation ==="

  vcom -2008 $Path_VHDL/fir_filter_pkg.vhd
  vcom -2008 $Path_VHDL/fir_filter.vhd
  puts "VHDL files compiled successfully"
}

#------------------------------------------------------------------------------
proc sv_compil { } {
  global Path_TB
  puts "\n=== SystemVerilog Compilation ==="

  vlog -sv $Path_TB/fir_filter_tb.sv
  puts "SystemVerilog testbench compiled successfully"
}

#------------------------------------------------------------------------------
proc sim_fir_filter { } {
  puts "\n=== Starting FIR Filter Simulation ==="
  vsim -t 1ns work.fir_filter_tb
  add wave -r *
  wave refresh
  run -all
}

#------------------------------------------------------------------------------
proc compile_all { } {
  vhdl_compil
  sv_compil
}

#------------------------------------------------------------------------------
proc do_all_fir { } {
  compile_all
  sim_fir_filter
}


#------------------------------------------------------------------------------
proc help { } {
  puts "\n=== FIR Filter Simulation Script ==="
  puts "Usage:"
  puts "  do sim.do"
  puts ""
  puts "Commands:"
  puts "  comp_vhdl     : Compile VHDL files only"
  puts "  comp_sv       : Compile SystemVerilog testbenches"
  puts "  comp_all      : Compile all VHDL and SystemVerilog files"
  puts "  sim_fir       : Run fir_filter_tb simulation"
  puts "  all_fir       : Compile and run fir_filter_tb"
  puts "  help          : Print this help message"
  puts ""
  puts "Default (no argument): Compile all and run fir_filter_tb"
}

## MAIN #######################################################################

# Compile folder --------------------------------------------------------
if {[file exists work] == 0} {
  mkdir work
  vlib work
  vmap work work
}

set Path_VHDL     "../src_vhdl"
set Path_TB       "../src_tb"

global Path_VHDL
global Path_TB

# Process command line arguments ----------------------------------------
if {$argc > 0} {
  set cmd $1
  if {[string compare $cmd "comp_vhdl"] == 0} {
    vhdl_compil
  } elseif {[string compare $cmd "comp_sv"] == 0} {
    sv_compil
  } elseif {[string compare $cmd "comp_all"] == 0} {
    compile_all
  } elseif {[string compare $cmd "sim_fir"] == 0} {
    sim_fir_filter
  } elseif {[string compare $cmd "all_fir"] == 0} {
    do_all_fir
  } elseif {[string compare $cmd "help"] == 0} {
    help
  } else {
    puts "Unknown command: $cmd"
    help
  }
} else {
  # Default: compile all and run FIR simulation
  do_all_fir
}