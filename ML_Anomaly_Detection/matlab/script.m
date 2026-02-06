port = input("Enter serial port (e.g., COM4): ", "s");

temp = log_temp_data(port);

histogram(temp, 25, "Normalization", "pdf");

mu = mean(temp);

sigma2 = var(temp, 1);
sigma = sqrt(sigma2);
hold on;

x = linspace(mu - 4*sigma, mu + 4*sigma, 400);
pdf = (1/(sqrt(2*pi)*sigma)) * exp(-(x-mu).^2 / (2*sigma2));

plot(x, pdf, 'LineWidth', 2);

hold off;


disp(mu);
disp(sigma);
disp(sigma2);