#!/bin/bash

# ###################################
# Shell script to remove output files
# Use: ./clean.sh .
# ####################################

echo "Cleaning files"
rm -f $1/*.o $1/FAIL* $1/INVALID_config* $1/VALID_config* 
rm -f $1/output.txt $1/log.cov *~ *.metrics
rm -f $1/config_temp.json
rm -f $1/diff_*_*
rm -f $1/m_*
rm -f $1/dd2_diff_*_*
rm -f $1/dd2_valid_*_*
rm -f $1/i_m_*
rm -f $1/log.cov
rm -f $1/log.dd
rm -f $1/counters.bc $1/cov* $1/temp_*.bc $1/demo_fn.bc
#rm -f $1/score.cov $1/sat.cov 
