function results = log_data(port)

s = serialport(port, 115200);
configureTerminator(s, "LF");
pause(0.5);

NUM_SCANS = 5;
MIN_MACS  = 3;
MAX_MACS  = 3;

% --- Place 1 ---
input("Place 1: press Enter to start logging...","s");
write(s, 'A', "char");
fprintf("Collecting data for Place 1...\n");
map1 = collect_place_all5(s, NUM_SCANS);   % mac -> 1xNUM_SCANS vector

% --- Place 2 ---
input("Place 2: press Enter to start logging...","s");
write(s, 'A', "char");
fprintf("Collecting data for Place 2...\n");
map2 = collect_place_all5(s, NUM_SCANS);

% --- MACs common to both places ---
k1 = map1.keys; 
k2 = map2.keys; 

common = intersect(k1, k2);
nCommon = numel(common);

if nCommon < MIN_MACS
    fprintf(2, ['ERROR: Only %d common MAC address(es) found between the two locations.\n' ...
                'Please collect data at another location.\n'], nCommon);
    results = table();   % return empty table
else
    % ---- Pick MACs with strongest average RSSI (over both places) ----
    avgRSSI = zeros(nCommon,1);
    for i = 1:nCommon
        key = common{i};
        v1  = map1(key);
        v2  = map2(key);
        avgRSSI(i) = mean([v1(:); v2(:)]);   % average over all scans, both places
    end

    % Sort by average RSSI (descending: less negative = stronger)
    [~, idxSort] = sort(avgRSSI, 'descend');
    idxKeep      = idxSort(1:min(MAX_MACS, nCommon));
    commonSel    = common(idxKeep);
    n            = numel(commonSel);

    % --- Build output table with the selected MACs ---
    MAC     = string(commonSel(:));
    RSSI_L1 = zeros(n, NUM_SCANS);
    RSSI_L2 = zeros(n, NUM_SCANS);

    for i = 1:n
        key = commonSel{i};
        RSSI_L1(i,:) = map1(key);
        RSSI_L2(i,:) = map2(key);
    end

    % Keep order = strongest to weakest; no sort by MAC
    results = table(MAC, RSSI_L1, RSSI_L2);
end

% --- Close serial port ---
flush(s);
clear s

end

% ---------- Local function ----------
function mapOut = collect_place_all5(s, NUM_SCANS)
    % Reads lines until 'SS'
    % Stores up to NUM_SCANS RSSI values per MAC in order of arrival.
    agg = containers.Map('KeyType','char','ValueType','any');
    currentScan = 0;  % for user feedback (1..NUM_SCANS)

    while true
        line = readline(s);
        line = strtrim(line);
        if strcmp(line, "SS")
            break;
        end

        parts = split(line, ",");
        if numel(parts) ~= 2
            continue;
        end

        macStr  = parts{1};
        rssiVal = str2double(parts{2});
        if isnan(rssiVal)
            continue;
        end

        if ~isKey(agg, macStr)
            agg(macStr) = nan(1, NUM_SCANS);   % initialize NUM_SCANS-slot vector
        end
        
        v = agg(macStr);
        idx = find(isnan(v), 1, 'first'); % next available slot
        
        if ~isempty(idx)
            v(idx) = rssiVal;
            agg(macStr) = v;

            % --- Display scan index progress (only when we see a new scan index) ---
            if idx > currentScan
                currentScan = idx;
                fprintf('Scan %d of %d received.\n', currentScan, NUM_SCANS);
            end
        end
    end

    % Only keep MACs that received all NUM_SCANS values
    keysMAC = agg.keys;
    mapOut = containers.Map('KeyType','char','ValueType','any');

    for i = 1:numel(keysMAC)
        k = keysMAC{i};
        v = agg(k);
        if all(~isnan(v))
            mapOut(k) = v;
        end
    end
end
