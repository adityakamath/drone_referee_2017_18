function rebuild = libfieldrebuildcheck(varargin)

if(nargin == 1 && strcmp(varargin{1}, 'sim') == 1)
    default_field = get_global_par('DEFAULTSIMFIELD');
else
    default_field = get_global_par('DEFAULTFIELDSELECTION');
end

curdir1 = cd;

cd /home/robocup/svn/trunk/src/Turtle2/Global_par

rebuild     = -1;
file        = 'libfield.txt';
fid         = fopen(file, 'r+');

if fid == -1
    % libfield.txt does not exist, rebuild needed, return 1
    rebuild = 1;
else
    % Read against which field the libs have been build last time from
    % libfield.txt
    libfield = fgetl(fid);
    fclose(fid);
    
    if isequal(libfield, default_field)
        rebuild = 0;
    else
        rebuild = 1;
    end
end

if rebuild == 1
    % Rebuild libs
    disp(['Rebuilding libs using field: ', default_field]);
    make_genfunlib
    make_mu_image
    make_mu
    libfieldrebuildset(default_field)
end

cd(curdir1);


