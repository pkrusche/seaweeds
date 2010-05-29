set INPUT=Berti_

bin\WindowAlignment_BSP_win32_default_release.exe benchmark\alignmentplots\%INPUT%a.txt benchmark\alignmentplots\%INPUT%b.txt --threads 1 result_1.txt profile_1_a.txt profile_1_b.txt
bin\WindowAlignment_BSP_win32_default_release.exe benchmark\alignmentplots\%INPUT%a.txt benchmark\alignmentplots\%INPUT%b.txt --threads 2 result_2.txt profile_2_a.txt profile_2_b.txt
bin\WindowAlignment_BSP_win32_default_release.exe benchmark\alignmentplots\%INPUT%a.txt benchmark\alignmentplots\%INPUT%b.txt --threads 3 result_3.txt profile_3_a.txt profile_3_b.txt
bin\WindowAlignment_BSP_win32_default_release.exe benchmark\alignmentplots\%INPUT%a.txt benchmark\alignmentplots\%INPUT%b.txt --threads 4 result_4.txt profile_4_a.txt profile_4_b.txt

mpiexec -n 2 bin\WindowAlignment_BSP_win32_default_release.exe benchmark\alignmentplots\%INPUT%a.txt benchmark\alignmentplots\%INPUT%b.txt result_2x4.txt profile_2x4_a.txt profile_2x4_b.txt
mpiexec -n 3 bin\WindowAlignment_BSP_win32_default_release.exe benchmark\alignmentplots\%INPUT%a.txt benchmark\alignmentplots\%INPUT%b.txt result_3x4.txt profile_3x4_a.txt profile_3x4_b.txt
mpiexec -n 4 bin\WindowAlignment_BSP_win32_default_release.exe benchmark\alignmentplots\%INPUT%a.txt benchmark\alignmentplots\%INPUT%b.txt result_4x4.txt profile_4x4_a.txt profile_4x4_b.txt

diff result_1.txt result_2.txt
diff result_1.txt result_3.txt
diff result_1.txt result_4.txt

diff result_1.txt result_2x4.txt
diff result_1.txt result_3x4.txt
diff result_1.txt result_4x4.txt

diff profile_1_a.txt profile_2_a.txt
diff profile_1_a.txt profile_3_a.txt
diff profile_1_a.txt profile_4_a.txt
diff profile_1_a.txt profile_2x4_a.txt
diff profile_1_a.txt profile_3x4_a.txt
diff profile_1_a.txt profile_4x4_a.txt

diff profile_1_b.txt profile_2_b.txt
diff profile_1_b.txt profile_3_b.txt
diff profile_1_b.txt profile_4_b.txt

diff profile_1_b.txt profile_2x4_b.txt
diff profile_1_b.txt profile_3x4_b.txt
diff profile_1_b.txt profile_4x4_b.txt
