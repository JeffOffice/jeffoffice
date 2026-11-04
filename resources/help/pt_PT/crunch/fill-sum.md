# Preencher células e somar

<!-- keywords: preencher, série, arrastar, alça, duplo clique, lista personalizada, Preenchimento Automático, Ctrl+D, Ctrl+R, para baixo, para a direita, para cima, para a esquerda, número, data, dia da semana, mês, ano, incremento, parar, copiar, Soma, soma, média, contar, máximo, mínimo, Alt+igual, total -->

## Preencher

O preenchimento continua o que começou. Selecione as células que preencheu e arraste o pequeno quadrado no canto da seleção (a **alça de preenchimento**) ao longo de uma coluna ou de uma linha, e depois largue. Um contorno tracejado mostra até onde vai o preenchimento.

| Começa com | O preenchimento dá |
| --- | --- |
| 1, 2 | 3, 4, 5, e assim sucessivamente: o incremento entre eles continua |
| Um número único, como 5 | 5, 5, 5: cópias (mantenha Ctrl premido ao arrastar para obter a opção contrária) |
| Uma data | Os dias seguintes (use o botão **Opções de Preenchimento**, ou **Preencher > Série**, para dias úteis, meses ou anos) |
| Seg, ou Janeiro | Ter, Qua, ... ou Fevereiro, Março, ... no seu idioma |
| Artigo 1 | Artigo 2, Artigo 3, ... |
| Uma palavra de uma lista personalizada, como Pequeno | A palavra seguinte da lista e, após a última, volta à primeira |
| Uma fórmula | A fórmula copiada, com as suas referências deslocadas como numa colagem |

Faça duplo clique na alça de preenchimento para preencher para baixo até onde chegam os dados ao lado da seleção. O JeffCrunch analisa primeiro a coluna à esquerda e depois a coluna à direita. Para na primeira célula vazia dessa coluna.

## Listas personalizadas

Uma **lista personalizada** é uma sequência de palavras que o Preenchimento percorre, como os dias da semana. Para criar uma, clique em **Ficheiro > Opções > Listas Personalizadas**. Escreva uma lista por linha, com vírgulas entre as palavras, por exemplo `Pequeno, Médio, Grande`. Clique em **OK**. Agora escreva **Pequeno** numa célula e arraste a alça de preenchimento: as células são preenchidas com Médio, Grande, Pequeno, e assim sucessivamente. As maiúsculas e minúsculas não importam quando o JeffCrunch procura uma palavra numa lista, e as novas células copiam as maiúsculas da célula de onde começou. Uma lista precisa de pelo menos duas palavras. Os dias da semana e os meses já estão incluídos.

**Preencher > Para Baixo** (**Ctrl+D**) e **Preencher > Para a Direita** (**Ctrl+R**) copiam a primeira linha ou coluna da seleção para o resto. **Preencher > Para Cima** e **Preencher > Para a Esquerda** trabalham a partir da última. **Preencher > Série** pede o tipo (linear, crescimento, data), a unidade de uma data, o incremento e o valor de paragem. Cada célula preenchida adota o formato da célula que a precede. Uma célula que não pode ser alterada (parte de uma matriz, uma célula bloqueada ou o cabeçalho de uma tabela) interrompe o preenchimento, e nada muda.

Depois de arrastar a alça de preenchimento, aparece o pequeno botão **Opções de Preenchimento** no canto do que foi preenchido. O respetivo menu repete o preenchimento de outra forma, e o preenchimento continua a ser um passo do Anular: **Copiar Células**, **Preencher Série**, **Preencher Apenas Formatação** (as células mantêm o que contêm e adotam o formato), **Preencher Sem Formatação** (as células mantêm o seu próprio formato) e, para datas, **Preencher Dias**, **Preencher Dias Úteis**, **Preencher Meses** e **Preencher Anos**. O botão desaparece quando fizer outra ação.

Um preenchimento não entra nem sai de células unidas, porque deixaria valores em células que a área unida oculta. Separe-as primeiro.

**Preencher > Justificar** distribui o texto das células selecionadas de uma coluna por linhas tão largas como a coluna, nas células a partir da primeira para baixo; as restantes células da seleção ficam vazias. Se o texto precisar de mais células do que as selecionadas, o JeffCrunch pergunta se deve utilizar as células abaixo. Funciona apenas com texto.

**Preencher > Entre Folhas de Cálculo** copia as células selecionadas para as mesmas células das folhas que marcar numa lista: tudo, apenas o conteúdo ou apenas os formatos. É um passo do Anular.

## Soma

**Base > Soma** (**Alt e o sinal de igual**) coloca um total na célula selecionada. Analisa os números acima da célula; se não houver nenhum, analisa os números à sua esquerda. A fórmula é colocada na célula para que a verifique, e **Enter** a mantém. A seta ao lado de Soma apresenta os outros totais: **Média**, **Quantos Números**, **Máx** e **Mín**.

Se selecionar primeiro um bloco, cada coluna do bloco recebe a sua própria fórmula nas células vazias por baixo (ou, para uma única linha, à direita).
