#!/bin/bash
#SBATCH -J soma
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --ntasks-per-node=4
#SBATCH --threads-per-core=1
#SBATCH --time=0:10:00
#SBATCH --output soma.output
#
module purge
module load openmpi/1.6.5
ltps
#source /gpfs/home/closier/SOMAFOAM/etc/bashrc
#source /gpfs/softs/contrib/apps/somaFoam/20230728/etc/bashrc
#----------------------------------------------------------
# Launch job
#----------------------------------------------------------
OF_APP=somaFoam
## run
mpirun -np $SLURM_NTASKS $OF_APP -parallel > logfile 2>&1
#srun $OF_APP > logfile 2>&1
#$OF_APP
