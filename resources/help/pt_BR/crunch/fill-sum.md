# Preencher células e somá-las
<!-- keywords: preencher, preenchimento, série, sequência, arrastar, alça, alça de preenchimento, AutoPreenchimento, Ctrl+D, Ctrl+R, para baixo, para a direita, para cima, para a esquerda, número, data, dia da semana, mês, ano, incremento, limite, copiar, AutoSoma, soma, somar, média, contagem, contar, máximo, mínimo, Alt+igual, total -->

## Preencher

O preenchimento continua o que você começou. Selecione as células que você preencheu e arraste o pequeno quadrado no canto da seleção (a **alça de preenchimento**) ao longo de uma coluna ou linha, depois solte. Uma borda tracejada mostra até onde o preenchimento vai.

| Você começa com | O preenchimento gera |
| --- | --- |
| 1, 2 | 3, 4, 5 e assim por diante: o incremento entre eles continua |
| Um único número, como 5 | 5, 5, 5: cópias (mantenha Ctrl pressionada ao arrastar para obter o contrário) |
| Uma data | Os dias seguintes (use **Preencher > Série** para dias da semana, meses ou anos) |
| Seg, ou janeiro | Ter, Qua, ... ou fevereiro, março, ... no seu idioma |
| Item 1 | Item 2, Item 3, ... |
| Uma fórmula | A fórmula copiada, com as referências deslocadas como em uma colagem |

**Preencher > Para Baixo** (**Ctrl+D**) e **Preencher > Para a Direita** (**Ctrl+R**) copiam a primeira linha ou coluna da seleção para o restante. **Preencher > Para Cima** e **Preencher > Para a Esquerda** partem da última. **Preencher > Série** pede o tipo (linear, crescimento, data), a unidade de data, o incremento e o limite. Cada célula preenchida recebe a formatação da célula que ela segue. Uma célula que não pode ser alterada (parte de uma matriz, uma célula bloqueada ou o cabeçalho de uma tabela) interrompe o preenchimento, e nada é alterado.

## AutoSoma

**Página Inicial > AutoSoma** (**Alt e o sinal de igual**) insere um total na célula selecionada. Ela verifica os números acima da célula; se não houver nenhum, os números à esquerda. A fórmula é inserida na célula para você conferir, e **Enter** a mantém. A seta ao lado de AutoSoma oferece os outros totais: **Média**, **Contar Números**, **Máx.** e **Mín.**

Se você selecionar um bloco antes, cada coluna do bloco recebe a própria fórmula nas células vazias abaixo dela (ou, no caso de uma única linha, à direita).
