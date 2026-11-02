# Preencher células e somá-las
<!-- keywords: preencher, preenchimento, série, sequência, arrastar, alça, Preenchimento Automático, Ctrl+D, Ctrl+R, para baixo, para a direita, para cima, para a esquerda, número, data, dia útil, mês, ano, incremento, valor limite, copiar, Soma, soma, somar, média, contar, máximo, mínimo, Alt+igual, total -->

## Preencher

O preenchimento continua o que começou. Selecione as células que preencheu e arraste o pequeno quadrado no canto da seleção (a **alça de preenchimento**) ao longo de uma coluna ou linha e solte. Um limite tracejado mostra até onde irá o preenchimento.

| Começa com | O preenchimento dá |
| --- | --- |
| 1, 2 | 3, 4, 5, e assim por diante: o incremento entre eles continua |
| Um único número, como 5 | 5, 5, 5: cópias (mantenha premida a tecla Ctrl enquanto arrasta para obter a opção contrária) |
| Uma data | Os dias seguintes (utilize o botão **Opções de Preenchimento**, ou **Preencher > Série**, para dias úteis, meses ou anos) |
| Seg ou janeiro | Ter, Qua, ... ou fevereiro, março, ... no seu idioma |
| Item 1 | Item 2, Item 3, ... |
| Uma fórmula | A fórmula copiada, com as referências deslocadas como numa colagem |

**Preencher > Para baixo** (**Ctrl+D**) e **Preencher > Para a direita** (**Ctrl+R**) copiam a primeira linha ou coluna da seleção para as restantes. **Preencher > Para cima** e **Preencher > Para a esquerda** trabalham a partir da última. **Preencher > Série** pede o tipo (linear, crescimento, data), a unidade de data, o valor do incremento e onde parar. Cada célula preenchida adota o formato da célula que a precede. Uma célula que não pode ser alterada (parte de uma matriz, uma célula bloqueada ou o cabeçalho de uma tabela) interrompe o preenchimento e nada é alterado.

Depois de arrastar a alça de preenchimento, o pequeno botão **Opções de Preenchimento** aparece no canto da área preenchida. O respetivo menu refaz o preenchimento de outra forma, e o preenchimento continua a ser um único passo de Anular: **Copiar Células**, **Preencher Série**, **Preencher Apenas Formatação** (as células mantêm o conteúdo e adotam o formato), **Preencher Sem Formatação** (as células mantêm o seu próprio formato) e, para datas, **Preencher Dias**, **Preencher Dias Úteis**, **Preencher Meses** e **Preencher Anos**. O botão desaparece quando faz outra coisa qualquer.

Um preenchimento não entra nem sai de células unidas, porque deixaria valores em células ocultas pela área unida. Anule primeiro a união das células.

**Preencher > Justificar** distribui o texto das células selecionadas de uma coluna por linhas com a largura da coluna, nas células a partir da primeira para baixo; as células da seleção que sobrarem são esvaziadas. Se o texto precisar de mais células do que as selecionadas, o JeffCrunch pergunta se deve utilizar as células abaixo. Só funciona com texto.

**Preencher > Entre Folhas de Cálculo** copia as células selecionadas para as mesmas células das folhas que assinalar numa lista: tudo, apenas o conteúdo ou apenas os formatos. É um único passo de Anular.

## Soma

**Base > Soma** (**Alt e o sinal de igual**) coloca um total na célula que selecionar. Analisa os números acima da célula; se não existirem, os números à esquerda. A fórmula é colocada na célula para que a verifique, e **Enter** confirma-a. A seta junto a Soma dá acesso aos outros totais: **Média**, **Contar Números**, **Máx.** e **Mín.**

Selecione primeiro um bloco, e cada coluna do bloco recebe a sua própria fórmula nas células vazias abaixo (ou, no caso de uma única linha, à direita).
