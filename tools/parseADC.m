DataFile = 'log7.csv';
VoltageScaling = 352/2;
CurrentScaling = 1/0.066;

rawData = csvread(DataFile);
V = rawData(:,4);
I = rawData(:,3);

Vc = ((V - mean(V)) * 10 / (2^16)) * VoltageScaling;
Ic = ((I - mean(I)) * 10 / (2^16));

sprintf('Voltage: %fVrms\nCurrent: %fIrms\nPower: %fW', rms(Vc), rms(Ic), mean(Ic.*Vc))

figure;
%subplot(2,1,1);
plot([0:(size(I,1)-1)]/8000,I);
xlabel('Time (s)');
ylabel('Current (I)');
%subplot(2,1,2);
%plot([0:(size(Ic,1)-1)]/8000,Ic);
%xlabel('Time (s)');
%ylabel('Current (V)');
