function vers = eegplugin_BCI2000import(fig, trystrs, catchstrs)

    vers = 'BCI2000import1.00';
    if nargin < 3
        error('eegplugin_BCI2000import requires 3 arguments');
    end;
    
    % add folder to path
    % ------------------
    if ~exist('BCI2000import')
        p = which('eegplugin_BCI2000import.m');
        p = p(1:findstr(p,'eegplugin_BCI2000import.m')-1);
        addpath([ p 'BCI2000import1.00' ] );
    end;
    
    % find import data menu
    % ---------------------
    menu = findobj(fig, 'tag', 'import data');
    
    % menu callbacks
    % --------------
    comcnt = [ trystrs.no_check '[EEG] = BCI2000import;'     catchstrs.new_and_hist ];
    
    % create menus
    % ------------
    uimenu( menu, 'label', 'BCI2000 Data', 'callback', comcnt, 'separator', 'on' );