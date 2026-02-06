port = input("Enter serial port (e.g., COM4): ", "s");
baud = 115200;

s = serialport(port, baud);
configureTerminator(s, "LF");
s.Timeout = 5;         
flush(s);

pause(1.0);            

maxSamples = 2000;
nbins = 25;
updateEvery = 10;

data = nan(maxSamples, 1);
n = 0;

fig = figure("Name","Live Histogram + Normal Fit","NumberTitle","off");
ax = axes(fig);
hHist = histogram(ax, [], nbins, "Normalization", "pdf");
hold(ax, "on");
hPdf = plot(ax, nan, nan, "LineWidth", 2);
hold(ax, "off");
grid(ax, "on");
xlabel(ax, "Value");
ylabel(ax, "PDF");

while ishandle(fig)
    
    try
        line = readline(s);    
    catch
   
        continue;
    end

    if isempty(line)
        continue;
    end
    line = string(line);
    line = strtrim(line);

    val = str2double(line);
    if isnan(val)
        continue;
    end

    n = n + 1;
    idx = mod(n-1, maxSamples) + 1;
    data(idx) = val;
    
    if mod(n, updateEvery) == 0
        valid = data(~isnan(data));
        if numel(valid) >= 5
            hHist.Data = valid;

            mu = mean(valid);
            sigma2 = var(valid, 1);
            sigma = sqrt(sigma2);

            xgrid = linspace(mu - 4*sigma, mu + 4*sigma, 400);
            pdf = (1/(sqrt(2*pi)*sigma)) * exp(-(xgrid-mu).^2 / (2*sigma2));

            set(hPdf, "XData", xgrid, "YData", pdf);
            title(ax, sprintf("n=%d  mu=%.4f  sigma=%.4f", n, mu, sigma));

            drawnow limitrate;
        end
    end
end

clear s;
