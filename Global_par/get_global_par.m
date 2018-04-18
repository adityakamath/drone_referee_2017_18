%
% get_global_par - gets Techunited global parameter
%
% usage: parval=get_global_par(parname)
%
% parname - name of global parameter
% parval - value of global parameter
%
% example: max_turtles=get_global_par('MAX_TURTLES')

%% @file
% gets Techunited global parameter
%
% usage: parval=get_global_par(parname)
% @param parname - name of global parameter
% @return parval - value of global parameter
%
% example: max_turtles=get_global_par('MAX_TURTLES')

function parval=get_global_par(varargin)

% get value of parameter named parname
parname=varargin{1};
if nargin>1,
    fn=varargin{2};
else fn='global_par.h';
end

% open global parameter file
filenamedir = mfilename('fullpath');
ind = max([findstr(filenamedir,'/') findstr(filenamedir,'\')]);
filenamedir = filenamedir(1:ind);
cmd=['fid=fopen(''' filenamedir fn ''');'];
eval(cmd);

if fid==-1
    error('Cannot open global parameter file.');
end

% look for global parameter with name parname
name=' ';
n=1;
while ~strcmp(name, parname) & n>0
    ch=char(0);
    while ~strcmp(ch, '#define')  & n>0
        [ch, n]=fscanf(fid, '%s', 1);
    end

    if n>0
        name=fscanf(fid, '%s', 1);
    end
end

if n>0
    try
    maindef=fscanf(fid, '%s', 1);
    maindef=strrep(maindef,'dmax','max');
    parval=eval(maindef);
    fclose(fid);
    catch ERR
        if strcmp(ERR.identifier,'MATLAB:UndefinedFunction')
            scChar=39; % ASCII equivalent of '
            ind=find(ERR.message==scChar);
            nesteddef=ERR.message(ind(1)+1:ind(2)-1);
            eval(strcat(nesteddef,'=get_global_par(',scChar,nesteddef,scChar,');'));
            parval=eval(maindef);
        end
    end
else
    fclose(fid);
    error(['Global parameter : ',parname,' does not exist.']);
end
