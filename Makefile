export ROOT       = $(CURDIR)
#export CIRCUIT    ?= ripple_carry
#export CIRCUIT    ?= trunc_adder
export CIRCUIT    ?= trunc_adder_SH
export FREQ_MHZ   ?= 100
export TECH       ?= 45
export OP_CORNER  ?= slow
export RTL_DIR    = ${ROOT}/Verilog/descriptions
       TESTS_DIR  = ${ROOT}/Verilog/testbenches

GUI        ?= 0
TB         ?= 1
MUL        ?= 0
GUI        ?= 0
TESTS      ?= 16
FLAGS_X += -access +rwc +define+TB
FLAGS_I += -g2012 -o testbench

ifeq ($(GUI),1)
	FLAGS_X += -gui
endif

ifneq ($(TESTS),16)
	FLAGS_X += +define+TESTS_NUM=$(TESTS)
	FLAGS_I += -DTESTS_NUM=${TESTS}
endif

VERILOG_PATH = /home/tools/CIRCUITkits/cadence/GPDK045/gsclib045_all_v4.4/gsclib045/verilog/slow_vdd1v0_basicCells.v

TREE ?= 0

trunc_adder_SH_xcelium:
	cd ${ROOT}/work && \
	xrun -v2001 ${RTL_DIR}/full_adder.v ${RTL_DIR}/ripple_carry.v ${RTL_DIR}/comparator.v ${RTL_DIR}/R0_Decision_Tree.v ${RTL_DIR}/R1_Decision_Tree.v ${RTL_DIR}/R2_Decision_Tree.v ${RTL_DIR}/R3_Decision_Tree.v ${RTL_DIR}/R4_Decision_Tree.v ${RTL_DIR}/Decision_Tree.v ${RTL_DIR}/trunc_adder_SH.v ${TESTS_DIR}/trunc_adder_SH_tb.sv $(FLAGS_X); \

Run_Logical_Synth:
	cd ${ROOT}/HDLs/HyperInference/synthesis/work && \
	genus -f $(ROOT)/HDLs/HyperInference/synthesis/scripts/synth.tcl -overwrite \