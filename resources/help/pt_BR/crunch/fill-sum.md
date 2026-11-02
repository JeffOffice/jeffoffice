# Preencher células e somá-las
<!-- keywords: preencher, preenchimento, série, arrastar, alça, alça de preenchimento, AutoPreenchimento, Ctrl+D, Ctrl+R, para baixo, para a direita, para cima, para a esquerda, número, data, dia útil, mês, ano, incremento, limite, copiar, Soma, soma, somar, média, contagem, contar, máximo, mínimo, Alt+igual, total -->

## Preencher

O preenchimento continua o que você começou. Selecione as células que você preencheu, arraste o pequeno quadrado no canto da seleção (a **alça de preenchimento**) ao longo de uma coluna ou linha e solte. Uma borda tracejada mostra até onde o preenchimento vai.

| Você começa com | O preenchimento gera |
| --- | --- |
| 1, 2 | 3, 4, 5 e assim por diante: o incremento entre eles continua |
| Um único número, como 5 | 5, 5, 5: cópias (mantenha Ctrl pressionada ao arrastar para obter o contrário) |
| Uma data | Os dias seguintes (use o botão **Opções de Preenchimento** ou **Preencher > Série** para dias úteis, meses ou anos) |
| Seg ou janeiro | Ter, Qua, ... ou fevereiro, março, ... no seu idioma |
| Item 1 | Item 2, Item 3, ... |
| Uma fórmula | A fórmula copiada, com as referências ajustadas como ao colar |

**Preencher > Para Baixo** (**Ctrl+D**) e **Preencher > Para a Direita** (**Ctrl+R**) copiam a primeira linha ou coluna da seleção para o restante. **Preencher > Para Cima** e **Preencher > Para a Esquerda** partem da última. **Preencher > Série** pede o tipo (linear, crescimento, data), a unidade de data, o incremento e onde parar. Cada célula preenchida recebe o formato da célula que ela segue. Uma célula que não pode ser alterada (parte de uma matriz, uma célula bloqueada ou o cabeçalho de uma tabela) interrompe o preenchimento, e nada é alterado.

Depois que você arrasta a alça de preenchimento, o pequeno botão **Opções de Preenchimento** aparece no canto da área preenchida. O menu dele refaz o preenchimento de outra forma, e o preenchimento continua sendo uma única etapa de Desfazer: **Copiar Células**, **Preencher Série**, **Preencher Somente Formatação** (as células mantêm o conteúdo e recebem o formato), **Preencher sem Formatação** (as células mantêm o próprio formato) e, para datas, **Preencher Dias**, **Preencher Dias Úteis**, **Preencher Meses** e **Preencher Anos**. O botão desaparece quando você faz qualquer outra coisa.

O preenchimento não entra nem sai de células mescladas, pois deixaria valores em células ocultas pela área mesclada. Desfaça a mesclagem primeiro.

**Preencher > Justificar** distribui o texto das células selecionadas de uma coluna em linhas da largura da coluna, nas células a partir da primeira para baixo; as células restantes da seleção são esvaziadas. Se o texto precisar de mais células do que as selecionadas, o JeffCrunch pergunta se deve usar as células abaixo. Funciona somente com texto.

**Preencher > Entre Planilhas** copia as células selecionadas para as mesmas células das planilhas que você marcar em uma lista: tudo, somente o conteúdo ou somente os formatos. É uma única etapa de Desfazer.

## Soma

**Página Inicial > Soma** (**Alt e o sinal de igual**) coloca um total na célula selecionada. Ela verifica os números acima da célula; se não houver nenhum, os números à esquerda. A fórmula é inserida na célula para você conferir, e **Enter** a mantém. A seta ao lado de Soma oferece os outros totais: **Média**, **Contar Números**, **Máx.** e **Mín.**

Se você selecionar um bloco antes, cada coluna do bloco recebe sua própria fórmula nas células vazias abaixo dela (ou, no caso de uma única linha, à direita).
