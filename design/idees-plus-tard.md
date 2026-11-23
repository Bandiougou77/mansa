# Idées pour plus tard

> Idées hors du périmètre de la vertical slice actuelle (règle 1 du `CLAUDE.md`).
> Format : date — idée — phase envisagée — qui l'a proposée.

| Date | Idée | Phase envisagée | Origine |
|---|---|---|---|
| 2026-11-23 | **Prompt griot v3** (`backend/resources/prompts/griot_djenne.v3.md`) : (1) ne plus reprendre les mots étrangers, toujours « ce mot étranger » (Q06, Q07, Q12, Q18, Q19, Q20 de l'éval v2 les répètent encore) ; (2) accord « le voleur » au masculin (Q14 disait « ta voleuse ») ; (3) 1 à 3 phrases maximum (plusieurs réponses en font 4) ; (4) contrôle de l'anglais renforcé dans `OutputFilter` (une phrase anglaise courte passe aujourd'hui). Valider par `php artisan griot:eval` (objectif 20/20 sans toucher au banc). | Phase 1 (bloc F, avant la relecture B7) | Développeur |
