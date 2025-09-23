# Istruzioni per Copilot

Questo file raccoglie le indicazioni che Copilot deve seguire ad ogni generazione di codice. Inserisci qui sotto i comandi che vanno ripetuti ogni volta, con una breve spiegazione del loro scopo.

## Comandi da eseguire sempre

- [ ] `Aggiorna intestazione file` - Prepara o aggiorna l'intestazione standard riportata sotto, sostituendo progetto, componente, file, revisione (incrementata) e data con i valori correnti al momento della creazione o modifica del codice.

```text
# /*********************************************************************************************
# PROJECT:          <PROJECT_NAME>
# COMPONENT:        <COMPONENT_PATH>
# FILE:             <CURRENT_FILE>
# AUTHOR:           Ing. Mirko Mirabella
#                   m.mirabella@neptunengineering.com
#                   www.neptunengineering.com
# REVISION:         v. <REVISION_NUMBER>
# DATE:             <CURRENT_DATE>
# **********************************************************************************************/
```

- [ ] `Aggiorna README` - Esegui lo script Python dedicato (`python tool/build_index.py`) per aggiornare tutti i file README interessati dal cambiamento.
- [ ] `Usa lingua inglese` - Scrivi tutto il codice, i commenti e le spiegazioni in inglese.
- [ ] `Usa stile di codice coerente` - Segui uno stile di codice coerente e leggibile, con indentazione, spaziatura e nomi di variabili chiari.
- [ ] `Includi commenti esplicativi` - Aggiungi commenti esplicativi per ogni sezione di codice, spiegando cosa fa e perché.
- [ ] `Verifica errori comuni` - Controlla il codice per errori comuni, come errori di sintassi, variabili non definite o problemi di logica.
- [ ] `Mantieni il contesto` - Assicurati che il codice generato sia coerente con il contesto del progetto e con le specifiche fornite.

## Modalita' d'uso
## Note aggiuntive


