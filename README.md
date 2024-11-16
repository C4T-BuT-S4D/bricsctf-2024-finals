[![check-services](https://github.com/C4T-BuT-S4D/bricsctf-2024-finals/actions/workflows/check-services.yml/badge.svg?branch=master&event=push)](https://github.com/C4T-BuT-S4D/bricsctf-2024-finals/actions/workflows/check-services.yml)

# BRICS+ CTF 2024 | Finals

The contest was held on November 16, 2024. Services and infrastructure by C4T BuT S4D. Organized by ITMO University and ACISO.

Repository contains source code of services, checkers and exploits.

## Results

![Top](scoreboard/top.png)

[Full scoreboard](scoreboard/full.png)

## Services

| Service                            | Language      | Checker                       | Sploits                      | Authors                                                                             |
|------------------------------------|---------------|-------------------------------|------------------------------|-------------------------------------------------------------------------------------|
| **[mole_vault](services/mole_vault/)**     | Go          | [Checker](checkers/mole_vault/)   | [Sploits](sploits/mole_vault/)   | [@pomo-mondreganto](https://github.com/pomo-mondreganto) |
| **[numbpy](services/numbpy/)** | Python & Go & C | [Checker](checkers/numbpy/) | [Sploits](sploits/numbpy/) | [@falamous](https://github.com/falamous)                                            |
| **[leakless](services/qrsas/)** | JavaScript & Java            | [Checker](checkers/qrsas/) | [Sploits](sploits/qrsas/) | [@jnovikov](https://github.com/jnovikov)                                    |
| **[notes](services/simple/)**       | C++    | [Checker](checkers/simple/)    | [Sploits](sploits/simple/)    | [@falamous](https://github.com/falamous)                                             |

## Infrastructure

- DevOps: [@pomo_mondreganto](https://github.com/pomo-mondreganto)
- Checksystem: [ForcAD](https://github.com/pomo-mondreganto/ForcAD)

## Writeups

- [mole_vault](/sploits/mole_vault/)
- [numbpy](/sploits/numbpy/)
- [qrsas](/sploits/qrsas/)
- [simple](/sploits/simple/)
