sweep_cpp = audioread("audio/sweep_output_cpp.wav");
sweep_matlab = audioread("audio/sweep_output_matlab.wav");

bass_cpp = audioread("audio/bass_output_cpp.wav");
bass_matlab = audioread("audio/bass_output_matlab.wav");

choir_cpp = audioread("audio/choir_output_cpp.wav");
choir_matlab = audioread("audio/choir_output_matlab.wav");


% sweep.wav
err_l = sweep_cpp(:, 1) - sweep_matlab(:, 1);
err_r = sweep_cpp(:, 2) - sweep_matlab(:, 2);

subplot(3, 1, 1);
plot(err_l, 'r');
hold on;
plot(err_r, 'b');
legend('L Channel', 'R Channel');
xlabel('Sample');
ylabel('Value');
title('Error of sweep.wav');

% bass.wav
err_l = bass_cpp(:, 1) - bass_matlab(:, 1);
err_r = bass_cpp(:, 2) - bass_matlab(:, 2);

subplot(3, 1, 2);
plot(err_l, 'r');
hold on;
plot(err_r, 'b');
legend('L Channel', 'R Channel');
xlabel('Sample');
ylabel('Value');
title('Error of bass.wav');

% choir.wav
err_l = choir_cpp(:, 1) - choir_matlab(:, 1);
err_r = choir_cpp(:, 2) - choir_matlab(:, 2);

subplot(3, 1, 3);
plot(err_l, 'r');
hold on;
plot(err_r, 'b');
legend('L Channel', 'R Channel');
xlabel('Sample');
ylabel('Value');
title('Error of choir.wav');