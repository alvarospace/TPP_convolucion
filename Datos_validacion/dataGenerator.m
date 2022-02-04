%% Generación de datos para validar el trabajo de TPP

% Matriz A
A = rand(8);
fprintf('\nMatriz de entrada A:\n');
disp(A);
writematrix(A);

% Filtro
F = rand(3);
fprintf('\nFiltro:\n');
disp(F);
writematrix(F);

% Resultado
B = conv2(A,flipud(fliplr(F)),"valid");
fprintf('\nMatriz resultado, B, sin padding:\n');
disp(B);
writematrix(B);



