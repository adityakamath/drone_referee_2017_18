function fieldnamelist = getfieldnamelist

fieldnamelist = cell(1,0);
k = dir('*.h');
for i=1:length(k)
    if ~isequal(k(i).name,'global_par.h')
        if ~isempty(findstr(k(i).name,'global_par_'))
            ind = findstr(k(i).name,'_');
            fieldnamelist = [fieldnamelist {k(i).name(ind(2)+1:end-2)}];
        end
    end
end