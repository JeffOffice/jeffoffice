# Tabelas
<!-- keywords: tabela, tabelas, referência estruturada, linha de cabeçalho, linha de totais, coluna calculada, em faixas, em tiras, estilo, aparência da tabela, botões de filtro, Ctrl+T, converter, células simples, redimensionar, nome da coluna, [@Coluna], Tabela1[Coluna] -->

Uma tabela é um bloco de células com uma linha de nomes sobre as colunas, tratado como uma coisa só: ela tem um nome, uma aparência, botões para filtrar e classificar as linhas e cresce quando você adiciona algo a ela. As fórmulas podem chamar as colunas pelo nome, em vez do endereço da célula.

## Criar uma tabela

Selecione as células, ou clique em uma célula dos dados, e escolha **Tabela** na guia **Inserir** (**Ctrl+T**). O JeffCrunch pergunta quais células a tabela abrange e se a primeira linha contém os nomes das colunas. Nomes que são números, datas ou fórmulas se tornam o texto que mostram, nomes vazios se tornam Coluna1, Coluna2 e assim por diante, e duas colunas com o mesmo nome são numeradas. Se não houver nomes, a tabela não terá linha de nomes, e as colunas serão chamadas Coluna1, Coluna2 e assim por diante nas fórmulas.

A galeria **Aparência da Tabela** (guia **Página Inicial** ou guia **Tabela**) mostra as aparências: claras, médias e escuras, cada uma em preto e nas seis cores do tema da pasta de trabalho. Escolher uma para células que não são uma tabela as transforma em tabela. A aparência é aplicada por baixo do que a própria célula tem: um preenchimento, bordas, uma cor de texto ou negrito definidos em uma célula são mantidos.

## A guia Tabela

A guia **Tabela** aparece enquanto o cursor está em uma tabela.

- **Nome** é o nome pelo qual as fórmulas chamam a tabela. Todas as fórmulas que usam o nome antigo acompanham a mudança. Um nome tem letras, dígitos, sublinhados e pontos, começa com uma letra ou um sublinhado e não pode ser o nome de outra tabela, um nome da pasta de trabalho ou um endereço de célula como A1.
- **Linha de Nomes**, **Linha de Totais**, **Linhas em Faixas**, **Colunas em Faixas**, **Primeira Coluna**, **Última Coluna** e **Botões de Filtro** são marcados ou desmarcados para a tabela.
- **Alterar Tamanho da Tabela…** pede as células que a tabela abrange. A primeira célula é mantida; linhas e colunas podem ser adicionadas na parte inferior e à direita, ou removidas.
- **Voltar a Células Simples** transforma a tabela em células comuns. Elas mantêm o conteúdo. Uma fórmula que citava a tabela ou as colunas dela passa a citar as células.

## O menu das células

Clique com o botão direito do mouse em uma célula de uma tabela e escolha **Tabela**: **Inserir Linha Acima**, **Inserir Linha Abaixo**, **Inserir Coluna à Esquerda**, **Inserir Coluna à Direita**, **Excluir Linhas da Tabela**, **Excluir Colunas da Tabela**, **Linha de Totais** e **Voltar a Células Simples**. Esses comandos inserem ou removem linhas e colunas inteiras da planilha, como Inserir e Excluir fazem; uma linha ou coluna inserida logo após o fim da tabela é incorporada a ela.

## Linha de totais

Ative **Linha de Totais**, e a linha abaixo da tabela terá a palavra Total na primeira coluna e um total na última. Cada célula da linha tem um botão: escolha o que o total da coluna mostra (a média, uma contagem, quantos números há, o maior valor, o menor, a soma, o desvio padrão, a variância ou **Fórmula Personalizada**). Um total é um SUBTOTAL da coluna, por isso ignora as linhas ocultadas por um filtro.

## Adicionar itens a uma tabela

- Digite na linha abaixo de uma tabela, ou na coluna ao lado dela, e a tabela cresce para incluir a célula. **Desfazer** remove o crescimento e mantém o que você digitou.
- Uma fórmula digitada em uma célula de uma coluna que está vazia nas demais células é colocada em todas as células da coluna: uma coluna calculada. Use **Desfazer** logo em seguida, e a fórmula fica apenas naquela célula. Uma fórmula digitada sobre uma coluna calculada altera a coluna inteira quando as outras células contêm a fórmula antiga; se algumas não contiverem, a célula fica isolada.
- As linhas inseridas dentro de uma tabela recebem as fórmulas das colunas calculadas. As colunas inseridas dentro dela recebem um nome na linha de nomes. Excluir linhas ou colunas dentro dela a torna menor; a linha de nomes e todas as colunas não podem ser excluídas de uma vez.

## Filtrar e classificar uma tabela

Os botões nos nomes filtram e classificam somente as linhas da tabela. Eles funcionam como os botões do filtro de uma planilha (consulte [Classificar e filtrar](sort-filter)). Uma tabela não pode ser filtrada enquanto um filtro da planilha, ou de outra tabela, abrange as mesmas linhas, já que cada um precisaria saber quais linhas são suas.

## Fórmulas que citam uma tabela

- `Sales[Amount]` são as células da coluna Amount, sem o nome e o total dela.
- `[@Amount]` é a célula da própria linha da fórmula e é gravada como `Sales[[#This Row],[Amount]]` no arquivo.
- `Sales[#Headers]`, `Sales[#Data]`, `Sales[#Totals]` e `Sales[#All]` são os nomes, os dados, os totais e a tabela inteira, e `Sales[[#Headers],[Amount]]` é uma coluna deles.
- `Sales[[Region]:[Amount]]` são as colunas de Region até Amount.

Digite `[` após o nome de uma tabela, e uma lista das colunas dela aparece. Enquanto você digita uma fórmula, clicar em uma célula de uma tabela grava uma referência como `Sales[@Amount]` ou `Sales[Amount]` (ao clicar em uma coluna inteira dos dados). Renomear uma coluna no cabeçalho altera o nome em todas as fórmulas; inserir e excluir colunas mantém as fórmulas apontadas para as colunas que citavam, e uma coluna excluída deixa #REF! nas fórmulas que a citavam.
