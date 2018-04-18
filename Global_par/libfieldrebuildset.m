function libfieldrebuildset(deffield)

curdir2 = cd;
cd /home/robocup/svn/trunk/src/Turtle2/Global_par

file        = 'libfield.txt';
fid         = fopen(file, 'w+');

% Write to which the currently selected field
fprintf(fid, '%s\n', deffield);

fclose(fid);

cd(curdir2);
