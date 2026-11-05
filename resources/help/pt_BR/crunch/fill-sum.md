# Preencher células e somar

<!-- keywords: preencher, série, arrastar, alça, clicar duas vezes, lista personalizada, AutoPreenchimento, Ctrl+D, Ctrl+R, para baixo, para a direita, para cima, para a esquerda, número, data, dia da semana, mês, ano, intervalo, parar, copiar, Soma, soma, média, contar, máximo, mínimo, Alt+igual, total, Preenchimento Relâmpago, Ctrl+E, padrão, dividir nomes -->

## Preencher

Preencher continua o que você começou. Selecione as células que você preencheu e arraste o pequeno quadrado no canto da seleção (a **alça de preenchimento**) ao longo de uma coluna ou de uma linha, e solte. Uma borda tracejada mostra até onde o preenchimento vai.

| Você começa com | O preenchimento oferece |
| --- | --- |
| 1, 2 | 3, 4, 5, e assim por diante: o intervalo entre eles continua |
| Um único número, como 5 | 5, 5, 5: cópias (mantenha Ctrl pressionado enquanto arrasta para obter a opção contrária) |
| Uma data | Os dias seguintes (use o botão **Opções de Preenchimento**, ou **Preencher > Série**, para dias úteis, meses ou anos) |
| Seg ou janeiro | Ter, Qua, ... ou fevereiro, março, ... no seu idioma |
| Item 1 | Item 2, Item 3, ... |
| Uma palavra de uma lista personalizada, como Pequeno | A próxima palavra da lista, e volta à primeira depois da última |
| Uma fórmula | A fórmula copiada, com suas referências deslocadas como em uma colagem |

Clique duas vezes na alça de preenchimento para preencher para baixo até onde forem os dados ao lado da seleção. O JeffCrunch verifica primeiro a coluna à esquerda e depois a coluna à direita. Ele para na primeira célula vazia dessa coluna.

## Listas personalizadas

Uma **lista personalizada** é uma sequência de palavras pela qual o Preencher passa, como os dias da semana. Para criar uma, clique em **Arquivo > Opções > Listas Personalizadas**. Digite uma lista em cada linha, com vírgulas entre as palavras, por exemplo `Pequeno, Médio, Grande`. Clique em **OK**. Agora digite **Pequeno** em uma célula e arraste a alça de preenchimento: as células serão preenchidas com Médio, Grande, Pequeno, e assim por diante. Letras maiúsculas e minúsculas não importam quando o JeffCrunch procura uma palavra em uma lista, e as novas células copiam as maiúsculas da célula de partida. Uma lista precisa de pelo menos duas palavras. Os dias da semana e os meses já vêm incluídos.

**Preencher > Para Baixo** (**Ctrl+D**) e **Preencher > Para a Direita** (**Ctrl+R**) copiam a primeira linha ou coluna da seleção sobre o restante. **Preencher > Para Cima** e **Preencher > Para a Esquerda** trabalham a partir da última. **Preencher > Série** pede o tipo (linear, crescimento, data), a unidade de uma data, o intervalo e o ponto de parada. Cada célula preenchida assume a formatação da célula que a precede. Uma célula que não pode ser alterada (parte de uma matriz, uma célula bloqueada ou o cabeçalho de uma tabela) interrompe o preenchimento, e nada muda.

Depois de arrastar a alça de preenchimento, o pequeno botão **Opções de Preenchimento** aparece no canto do que foi preenchido. O menu dele refaz o preenchimento de outra maneira, e o preenchimento continua sendo uma etapa do Desfazer: **Copiar Células**, **Preencher Série**, **Preencher Somente Formatação** (as células mantêm o que contêm e assumem a formatação), **Preencher sem Formatação** (as células mantêm a própria formatação) e, para datas, **Preencher Dias**, **Preencher Dias Úteis**, **Preencher Meses** e **Preencher Anos**. O botão desaparece quando você faz qualquer outra coisa.

O preenchimento não entra nem sai de células mescladas, porque deixaria valores em células que a área mesclada oculta. Desfaça a mesclagem antes.

**Preencher > Justificar** distribui o texto das células selecionadas de uma coluna em linhas com a largura da coluna, nas células a partir da primeira para baixo; as células restantes da seleção são esvaziadas. Se o texto precisar de mais células do que você selecionou, o JeffCrunch pergunta se deve usar as células abaixo. O comando funciona somente com texto.

**Preencher > Preenchimento Relâmpago** (**Ctrl+E**) preenche uma coluna para você a partir de um padrão que você mostra. Digite à mão o resultado desejado na primeira ou nas duas primeiras células de uma coluna, ao lado das células de origem. Depois, coloque o cursor nessa coluna e escolha Preenchimento Relâmpago. O JeffCrunch descobre como seus exemplos foram feitos a partir das células ao lado e preenche as células vazias da mesma maneira. Exemplo: a coluna A contém nomes como **Smith, Ann**. Em B1 você digita **Ann Smith**. O Preenchimento Relâmpago preenche B2 e as seguintes com os outros nomes invertidos da mesma maneira. Ele pode pegar uma palavra de uma célula, a primeira letra de uma palavra ou as primeiras ou últimas letras, em maiúsculas, minúsculas ou como estavam, e pode acrescentar um texto que é igual em todos os exemplos, como um espaço ou um ponto. Se não encontrar um padrão, ele avisa. Digite mais um ou dois exemplos e tente de novo. É uma etapa do Desfazer.

**Preencher > Em Todas as Planilhas** copia as células selecionadas para as mesmas células das planilhas que você marcar em uma lista: tudo, somente o conteúdo ou somente as formatações. É uma etapa do Desfazer.

## Soma

**Página Inicial > Soma** (**Alt e o sinal de igual**) coloca um total na célula selecionada. Ele examina os números acima da célula; se não houver nenhum, examina os números à esquerda dela. A fórmula é colocada na célula para você conferir, e **Enter** a mantém. A seta ao lado de Soma oferece os outros totais: **Média**, **Quantidade de Números**, **Máximo** e **Mínimo**.

Selecione um bloco primeiro, e cada coluna do bloco recebe a própria fórmula nas células vazias abaixo dela (ou, para uma única linha, à direita dela).
