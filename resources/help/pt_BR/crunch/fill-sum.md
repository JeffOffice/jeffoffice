# Preencher células e somar

<!-- keywords: preencher, série, arrastar, alça, clique duplo, lista personalizada, AutoPreenchimento, Ctrl+D, Ctrl+R, para baixo, para a direita, para cima, para a esquerda, número, data, dia da semana, mês, ano, intervalo, parar, copiar, Soma, soma, média, contagem, máximo, mínimo, Alt+igual, total, autopreenchimento, somar -->

## Preencher

Preencher continua o que você começou. Selecione as células que você preencheu e arraste o pequeno quadrado no canto da seleção (a **alça de preenchimento**) ao longo de uma coluna ou de uma linha, depois solte. Uma borda tracejada mostra até onde o preenchimento vai.

| Você começa com | O preenchimento gera |
| --- | --- |
| 1, 2 | 3, 4, 5, e assim por diante: o intervalo entre eles continua |
| Um único número, como 5 | 5, 5, 5: cópias (mantenha Ctrl pressionado ao arrastar para obter o resultado oposto) |
| Uma data | Os dias seguintes (use o botão **Opções de Preenchimento**, ou **Preencher > Série**, para dias da semana, meses ou anos) |
| Seg, ou Janeiro | Ter, Qua, ... ou Fevereiro, Março, ... no seu idioma |
| Item 1 | Item 2, Item 3, ... |
| Uma palavra de uma lista personalizada, como Pequeno | A próxima palavra da lista e, depois da última, volta à primeira |
| Uma fórmula | A fórmula copiada, com as referências deslocadas como em uma colagem |

Clique duas vezes na alça de preenchimento para preencher para baixo até onde vão os dados ao lado da seleção. O JeffCrunch verifica primeiro a coluna à esquerda e depois a coluna à direita. Ele para na primeira célula vazia dessa coluna.

## Listas personalizadas

Uma **lista personalizada** é uma sequência de palavras que o Preencher percorre, como os dias da semana. Para criar uma, clique em **Arquivo > Opções > Listas Personalizadas**. Digite uma lista em cada linha, com vírgulas entre as palavras, por exemplo `Pequeno, Médio, Grande`. Clique em **OK**. Agora digite **Pequeno** em uma célula e arraste a alça de preenchimento: as células serão preenchidas com Médio, Grande, Pequeno, e assim por diante. Letras maiúsculas não importam quando o JeffCrunch procura uma palavra em uma lista, e as novas células copiam as letras maiúsculas da célula de início. Uma lista precisa ter pelo menos duas palavras. Os dias da semana e os meses já vêm incluídos.

**Preencher > Para Baixo** (**Ctrl+D**) e **Preencher > Para a Direita** (**Ctrl+R**) copiam a primeira linha ou coluna da seleção sobre o restante. **Preencher > Para Cima** e **Preencher > Para a Esquerda** trabalham a partir da última. **Preencher > Série** pede o tipo (linear, crescimento, data), a unidade de uma data, o intervalo e onde parar. Cada célula preenchida recebe a formatação da célula que a antecede. Uma célula que não pode ser alterada (parte de uma matriz, uma célula bloqueada ou o cabeçalho de uma tabela) interrompe o preenchimento, e nada é alterado.

Depois de arrastar a alça de preenchimento, aparece o pequeno botão **Opções de Preenchimento** no canto do que foi preenchido. O menu dele refaz o preenchimento de outra forma, e o preenchimento continua sendo uma única etapa para Desfazer: **Copiar Células**, **Preencher Série**, **Preencher Somente Formatação** (as células mantêm o que contêm e recebem a formatação), **Preencher sem Formatação** (as células mantêm a própria formatação) e, para datas, **Preencher Dias**, **Preencher Dias Úteis**, **Preencher Meses** e **Preencher Anos**. O botão desaparece quando você faz qualquer outra coisa.

Um preenchimento não entra nem sai de células mescladas, pois deixaria valores em células que a área mesclada oculta. Desfaça a mesclagem antes.

**Preencher > Justificar** distribui o texto das células selecionadas de uma coluna em linhas com a largura da coluna, nas células a partir da primeira para baixo; as células restantes da seleção são esvaziadas. Se o texto precisar de mais células do que as selecionadas, o JeffCrunch pergunta se deve usar as células abaixo. O recurso funciona apenas com texto.

**Preencher > Entre Planilhas** copia as células selecionadas para as mesmas células das planilhas que você marcar em uma lista: tudo, apenas o conteúdo ou apenas as formatações. É uma única etapa para Desfazer.

## Somar

**Página Inicial > Soma** (**Alt e a tecla de igual**) coloca um total na célula selecionada. Ele considera os números acima da célula; se não houver nenhum, considera os números à esquerda. A fórmula é colocada na célula para você conferir, e **Enter** a mantém. A seta ao lado de Soma oferece os outros totais: **Média**, **Contar Números**, **Máx** e **Mín**.

Selecione primeiro um intervalo, e cada coluna do intervalo recebe a própria fórmula nas células vazias abaixo dela (ou, para uma única linha, à direita dela).
