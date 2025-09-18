# Esempi Simulink di Base

Modelli didattici per iniziare con Simulink.

## simple_oscillator

Sistema massa-molla-smorzatore semplice.

**Teoria**: Sistema del secondo ordine con equazione differenziale:
```
m*x'' + c*x' + k*x = F(t)
```

**Parametri**:
- Massa (m): 1 kg
- Smorzamento (c): 2 N⋅s/m  
- Rigidezza (k): 10 N/m

**Files**:
- `simple_oscillator_setup.m` - Script di configurazione
- `simple_oscillator.slx` - Modello Simulink (da creare)

**Uso**:
1. Eseguire `simple_oscillator_setup.m`
2. Aprire `simple_oscillator.slx`
3. Simulare per vedere la risposta all'impulso

**Risultati attesi**:
- Oscillazione smorzata
- Frequenza naturale ≈ 1.6 Hz
- Smorzamento sotto-critico