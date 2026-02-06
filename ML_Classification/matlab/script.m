%results = log_data("COM4");

alpha = 0.00001

matrix_x = [results.RSSI_L1 , results.RSSI_L2];

%{
matrix_x = [ ...
    -83  -83  -80  -81  -80   -71  -70  -71  -71  -71;
    -69  -68  -66  -67  -67   -58  -58  -58  -57  -58;
    -68  -68  -69  -70  -71   -51  -52  -52  -53  -52
];
%}

vector_y = [ +1, +1, +1, +1, +1, -1, -1, -1, -1, -1,]

theta_test = [0;0;0]

function grad = gradient(x, y, theta)
    grad = ((-y)*x)/(1+exp(y*(x' * theta)));
end

K = 1e5;
J_plots = zeros(K,1); 

for k = 1:K
    grad_total = zeros(3,1);
    for n = 1:10
        x_n = matrix_x(:, n);
        y_n = vector_y(n);
        grad_total = grad_total + gradient(x_n, y_n, theta_test);
    end
    theta_test = theta_test - alpha * grad_total ;

    J = 0;
    for n = 1:10
        x_n = matrix_x(:, n);
        y_n = vector_y(n);
        J = J + log(1 + exp(-y_n * (x_n' * theta_test)));
    end
    J_plots(k) = J;
end

disp(theta_test);

figure; clf; hold off;
plot(1:K, J_plots);
xlabel('k'); ylabel('J(\theta^{(k)})'); grid on;