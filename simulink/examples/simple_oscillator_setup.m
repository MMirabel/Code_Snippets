%% Setup script per modello Simple Oscillator
% Sistema massa-molla-smorzatore
%
% Configurazione parametri e workspace per simulazione

clear; clc;

%% Parametri del sistema
% Massa (kg)
m = 1.0;

% Coefficiente di smorzamento (N*s/m)
c = 2.0;

% Costante elastica della molla (N/m)  
k = 10.0;

% Forza di eccitazione (N)
F_amplitude = 1.0;

%% Parametri di simulazione
% Tempo di simulazione (s)
sim_time = 10.0;

% Passo di integrazione massimo (s)
max_step = 0.01;

%% Calcoli teorici
% Frequenza naturale non smorzata (rad/s)
omega_n = sqrt(k/m);

% Rapporto di smorzamento
zeta = c / (2 * sqrt(k*m));

% Frequenza naturale smorzata (rad/s)  
omega_d = omega_n * sqrt(1 - zeta^2);

% Periodo di oscillazione smorzata (s)
T_d = 2*pi / omega_d;

%% Visualizza parametri calcolati
fprintf('=== Parametri del Sistema ===\n');
fprintf('Massa (m): %.2f kg\n', m);
fprintf('Smorzamento (c): %.2f N*s/m\n', c);
fprintf('Rigidezza (k): %.2f N/m\n', k);
fprintf('\n=== Caratteristiche Dinamiche ===\n');
fprintf('Frequenza naturale: %.2f rad/s (%.2f Hz)\n', omega_n, omega_n/(2*pi));
fprintf('Rapporto di smorzamento: %.3f\n', zeta);
fprintf('Frequenza smorzata: %.2f rad/s (%.2f Hz)\n', omega_d, omega_d/(2*pi));
fprintf('Periodo smorzato: %.2f s\n', T_d);

if zeta < 1
    fprintf('Sistema: SOTTOSMORZATO (oscillatorio)\n');
elseif zeta == 1
    fprintf('Sistema: CRITICAMENTE SMORZATO\n');
else
    fprintf('Sistema: SOVRASMORZATO\n');
end

%% Condizioni iniziali
% Posizione iniziale (m)
x0 = 0.1;

% Velocità iniziale (m/s)  
v0 = 0.0;

fprintf('\n=== Condizioni Iniziali ===\n');
fprintf('Posizione iniziale: %.2f m\n', x0);
fprintf('Velocità iniziale: %.2f m/s\n', v0);

%% Verifica workspace
fprintf('\n=== Variabili create nel workspace ===\n');
vars = {'m', 'c', 'k', 'F_amplitude', 'sim_time', 'max_step', ...
        'omega_n', 'zeta', 'omega_d', 'T_d', 'x0', 'v0'};

for i = 1:length(vars)
    fprintf('%s = %.4f\n', vars{i}, eval(vars{i}));
end

fprintf('\nSetup completato! Ora puoi aprire simple_oscillator.slx\n');

%% Plot risposta teorica (opzionale)
if exist('plot_theory', 'var') && plot_theory
    t = 0:0.01:sim_time;
    
    % Risposta all'impulso per sistema sottosmorzato
    if zeta < 1
        x_theory = (F_amplitude/m) * (1/omega_d) * exp(-zeta*omega_n*t) .* sin(omega_d*t);
    end
    
    figure;
    plot(t, x_theory);
    title('Risposta teorica all''impulso - Sistema Massa-Molla-Smorzatore');
    xlabel('Tempo (s)');
    ylabel('Posizione (m)');
    grid on;
    
    % Aggiungi informazioni sul grafico
    text(0.7*sim_time, 0.8*max(x_theory), ...
         sprintf('\\omega_n = %.2f rad/s\n\\zeta = %.3f\nT_d = %.2f s', ...
         omega_n, zeta, T_d), ...
         'BackgroundColor', 'white');
end