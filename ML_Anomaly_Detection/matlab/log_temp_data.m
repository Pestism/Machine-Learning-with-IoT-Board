function temp = log_temp_data(port)
%
% Function for logging temperature data from the IoT board. On the IoT
% board the sketch LOG_TEMP_DATA.ino must be loaded. Make sure to close the 
% Arduino IDE before logging data to avoid conflicts handling the COM port. 
% The sample rate and number of samples to collect is controlled in the 
% LOG_TEMP_DATA.ino file. 
%
% Input: COM - comport to use. 
%
% Output: temp - vector with recorded temperature values
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


s = serialport(port, 115200, "Timeout", 120);
configureTerminator(s, "LF");
pause(0.5);

input("Press Enter to start logging...","s");
write(s, "A", "char");
temp = collect_temps(s);

flush(s);
clear s;

end


% ---------- Local function ----------
function temps = collect_temps(s)
    % Reads float values until "SS"
    T = [];  % dynamic array

    while true
        line = readline(s);
        line = strtrim(line);

        if strcmp(line, "SS")
            break;
        end

        val = str2double(line);
        if ~isnan(val)
            T(end+1) = val; %#ok<AGROW>
            numel(T)
        end
    end

    temps = T;   % return array of floats
end

