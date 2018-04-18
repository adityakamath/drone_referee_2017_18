function selectfield(defflag,selectfield)
%for defflag<=0, only global_par.h is updated (not the calibration data,
%nor the fieldmap)

curdir = cd;
cd /home/robocup/svn/trunk/src/Turtle2/Global_par

if nargin == 1 %use default field selected in global_par.h
    deffield=get_global_par('DEFAULTFIELDSELECTION');
    copyfieldparameters(['global_par_' deffield '.h'],'global_par.h')
    cd ../Vision
    copycalibrationdata(deffield)
    cd ./Tools/fieldmap_tool
    makefieldmap
elseif nargin == 2
    copyfieldparameters(selectfield,'global_par.h')
    if defflag>0,
        deffield=get_global_par('DEFAULTFIELDSELECTION');
        cd ../Vision
        copycalibrationdata(deffield)
        cd ./Tools/fieldmap_tool
        makefieldmap %1
    end
    
else %select from list
    gp=dir('global_par_*');
    sel=listdlg('ListString',{gp.name},'SelectionMode','single','name','select field','promptstring','select global_par data:');
    copyfieldparameters(gp(sel).name,'global_par.h')
    deffield=get_global_par('DEFAULTFIELDSELECTION');
    cd ../Vision
    copycalibrationdata(deffield)
    cd ./Tools/fieldmap_tool
    makefieldmap %1
end
cd(curdir)

function copycalibrationdata(fieldname)

for i=1:7
    cd(['Calibration_data_turtle' int2str(i)])
    try
        copyfile(['./fields/' fieldname '.tar'],['./' fieldname '.tar'],'f')
        delete('./*.mat')
        delete('./*.dat')
        [aa1,aa2]=system(['tar -xzf ' fieldname '.tar']);
        delete(['./' fieldname '.tar'])
        delete('./*.mat')
    catch
    end
    cd ..
end


function copyfieldparameters(source,destination)

data_s = getdata(source);
data_d = getdata(destination);

index_begin = 0;
for i=1:length(data_d)
    if isequal(data_d{i},'/*---field dependent parameters begin---*/')
        index_begin = i;
        break
    end
end
index_end = 0;
for i=1:length(data_d)
    if isequal(data_d{i},'/*---field dependent parameters end---*/')
        index_end = i;
        break
    end
end
if index_end*index_begin == 0
    error('global_par.h has not the proper field parameter format')
end
data_n = [data_d(1:index_begin),data_s,data_d(index_end:end)];
fid = fopen('temp.tmp','w');
for i=1:length(data_n)
    fprintf(fid,'%s\n',data_n{i});
end
fclose(fid);
movefile('temp.tmp',destination,'f');


function data_s = getdata(file)
fid=fopen(file);
if fid == -1
    error(['Could not open :',file]);
end
i_line = 1;
data_s={};
while 1
    tline = fgetl(fid);
    if ~ischar(tline)
        break
    else
        data_s{i_line}=tline;
        i_line=i_line+1;
    end
end
fclose(fid);
