# Preencher células e somar

<!-- keywords: preencher, série, arrastar, alça, duplo clique, lista personalizada, Preenchimento Automático, Ctrl+D, Ctrl+R, para baixo, para a direita, para cima, para a esquerda, número, data, dia da semana, mês, ano, incremento, parar, copiar, Soma, soma, média, contar, máximo, mínimo, Alt+igual, total, Preenchimento Relâmpago, Ctrl+E, padrão, separar nomes -->

## Preencher

O preenchimento continua aquilo que começou. Selecione as células que preencheu e arraste o pequeno quadrado no canto da seleção (a **alça de preenchimento**) ao longo de uma coluna ou de uma linha, e depois solte. Um contorno tracejado mostra até onde irá o preenchimento.

| Começa com | O preenchimento dá |
| --- | --- |
| 1, 2 | 3, 4, 5, e assim por diante: o incremento entre eles continua |
| Um único número, como 5 | 5, 5, 5: cópias (mantenha Ctrl premido ao arrastar para obter a opção contrária) |
| Uma data | Os dias seguintes (use o botão **Opções de Preenchimento**, ou **Preencher > Série**, para dias da semana, meses ou anos) |
| Seg, ou janeiro | Ter, Qua, ... ou fevereiro, março, ... no seu idioma |
| Item 1 | Item 2, Item 3, ... |
| Uma palavra de uma lista personalizada, como Pequeno | A palavra seguinte da lista, e de volta à primeira depois da última |
| Uma fórmula | A fórmula copiada, com as referências deslocadas como numa colagem |

Faça duplo clique na alça de preenchimento para preencher para baixo até onde forem os dados junto à seleção. O JeffCrunch analisa primeiro a coluna à esquerda e depois a coluna à direita. Para na primeira célula vazia dessa coluna.

## Listas personalizadas

Uma **lista personalizada** é uma sequência de palavras que o Preenchimento percorre, como os dias da semana. Para criar uma, clique em **Ficheiro > Opções > Listas Personalizadas**. Escreva uma lista em cada linha, com vírgulas entre as palavras, por exemplo `Pequeno, Médio, Grande`. Clique em **OK**. Agora escreva **Pequeno** numa célula e arraste a alça de preenchimento: as células são preenchidas com Médio, Grande, Pequeno, e assim por diante. As maiúsculas e minúsculas não importam quando o JeffCrunch procura uma palavra numa lista, e as novas células copiam as maiúsculas da célula de onde partiu. Uma lista precisa de pelo menos duas palavras. Os dias da semana e os meses já vêm incorporados.

**Preencher > Para Baixo** (**Ctrl+D**) e **Preencher > Para a Direita** (**Ctrl+R**) copiam a primeira linha ou coluna da seleção para o resto. **Preencher > Para Cima** e **Preencher > Para a Esquerda** trabalham a partir da última. **Preencher > Série** pede o tipo (linear, crescimento, data), a unidade de uma data, o incremento e o valor em que deve parar. Cada célula preenchida adota o formato da célula que a precede. Uma célula que não pode ser alterada (parte de uma matriz, uma célula bloqueada ou o cabeçalho de uma tabela) interrompe o preenchimento, e nada é alterado.

Depois de arrastar a alça de preenchimento, aparece o pequeno botão **Opções de Preenchimento** no canto do que foi preenchido. O seu menu repete o preenchimento de outra forma, e o preenchimento continua a ser um passo do Anular: **Copiar Células**, **Preencher Série**, **Preencher Apenas Formatação** (as células mantêm o que contêm e adotam o formato), **Preencher Sem Formatação** (as células mantêm o formato próprio) e, para datas, **Preencher Dias**, **Preencher Dias Úteis**, **Preencher Meses** e **Preencher Anos**. O botão desaparece quando faz outra coisa.

Um preenchimento não entra nem sai de células unidas, porque deixaria valores em células que a área unida oculta. Separe-as primeiro.

**Preencher > Justificar** coloca o texto das células selecionadas de uma coluna em linhas tão largas quanto a coluna, nas células a partir da primeira para baixo; as células restantes da seleção ficam vazias. Se o texto precisar de mais células do que as selecionadas, o JeffCrunch pergunta se deve usar as células abaixo. Funciona apenas com texto.

**Preencher > Preenchimento Relâmpago** (**Ctrl+E**) preenche uma coluna por si a partir de um padrão que lhe mostre. Escreva à mão o resultado pretendido nas primeiras uma ou duas células de uma coluna, ao lado das células de onde provém. Depois coloque o cursor nessa coluna e escolha Preenchimento Relâmpago. O JeffCrunch descobre como os seus exemplos foram feitos a partir das células ao lado e preenche as células vazias da mesma forma. Exemplo: a coluna A tem nomes como **Smith, Ann**. Em B1 escreve **Ann Smith**. O Preenchimento Relâmpago preenche B2 e as seguintes com os outros nomes invertidos da mesma forma. Pode retirar uma palavra de uma célula, a primeira letra de uma palavra, ou as primeiras ou últimas letras, em maiúsculas, minúsculas ou como estavam, e pode acrescentar texto que seja igual em todos os exemplos, como um espaço ou um ponto. Se não encontrar um padrão, avisa-o. Escreva mais um ou dois exemplos e tente de novo. É um passo do Anular.

**Preencher > Entre Folhas de Cálculo** copia as células selecionadas para as mesmas células das folhas que marcar numa lista: tudo, apenas o conteúdo ou apenas os formatos. É um passo do Anular.

## Soma

**Base > Soma** (**Alt e o sinal de igual**) coloca um total na célula selecionada. Procura os números acima da célula; se não houver nenhum, procura os números à sua esquerda. A fórmula é colocada na célula para que a verifique, e **Enter** a mantém. A seta junto a Soma dá os outros totais: **Média**, **Quantos Números**, **Máx** e **Mín**.

Selecione primeiro um bloco, e cada coluna do bloco recebe a sua própria fórmula nas células vazias por baixo dela (ou, para uma única linha, à direita).
