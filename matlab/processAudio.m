% sweep.wav
x = audioread("audio/sweep.wav");
y = zeros(size(x));

for i=1:length(x(1, :))
    y(:, i) = vibrato(x(:, i), 44100, 5, 50/44100);
end

audiowrite("audio/sweep_output_matlab.wav", y, 44100);

% bass.wav
x = audioread("audio/bass.wav");
y = zeros(size(x));

for i=1:length(x(1, :))
    y(:, i) = vibrato(x(:, i), 44100, 5, 50/44100);
end

audiowrite("audio/bass_output_matlab.wav", y, 44100);

% choir.wav
x = audioread("audio/choir.wav");
y = zeros(size(x));

for i=1:length(x(1, :))
    y(:, i) = vibrato(x(:, i), 44100, 5, 50/44100);
end

audiowrite("audio/choir_output_matlab.wav", y, 44100);