function selectgoal(selectgoal)

curdir = cd;
cd /home/robocup/svn/trunk/src/Turtle2/Global_par
copygoalparameters(selectgoal,'global_par.h')
cd(curdir)

end


function copygoalparameters(source,destination)

data_s = getdata(source);
data_d = getdata(destination);

index_begin = 0;
for i=1:length(data_d)
    if isequal(data_d{i},'/*---goal dependent parameters begin---*/')
        index_begin = i;
        break
    end
end
index_end = 0;
for i=1:length(data_d)
    if isequal(data_d{i},'/*---goal dependent parameters end---*/')
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

end

function data_s = getdata(file)
fid=fopen(file);
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
end