# Tabelas
<!-- keywords: tabela, tabelas, remover duplicatas, duplicatas, resumir, limpar estilo, referência estruturada, linha de cabeçalho, linha de totais, coluna calculada, faixas, linhas alternadas, estilo, aparência da tabela, novo estilo de tabela, estilo de tabela personalizado, botões de filtro, Ctrl+T, converter, células simples, redimensionar, nome da coluna, [@Coluna], Tabela1[Coluna] -->

Uma tabela é um bloco de células com uma linha de nomes sobre as colunas, tratado como um único elemento: tem um nome, uma aparência, botões para filtrar e classificar as linhas, e cresce quando você acrescenta dados a ela. As fórmulas podem usar as colunas da tabela pelo nome, em vez do endereço das células.

## Criar uma tabela

Selecione as células, ou clique em uma célula dos dados, e escolha **Tabela** na guia **Inserir** (**Ctrl+T**). O JeffCrunch pergunta quais células a tabela abrange e se a primeira linha contém os nomes das colunas. Nomes que são números, datas ou fórmulas passam a ser o texto que eles exibem, nomes vazios viram Coluna1, Coluna2, e assim por diante, e duas colunas com o mesmo nome são numeradas. Se não houver nomes, a tabela não tem linha de nomes, e as colunas se chamam Coluna1, Coluna2, e assim por diante nas fórmulas.

A galeria **Aparência da Tabela** (guia **Página Inicial** ou guia **Tabela**) mostra as aparências (**Limpar**, abaixo da galeria na guia **Tabela**, remove a aparência da tabela): clara, média e escura, cada uma em preto e nas seis cores do tema da pasta de trabalho. Escolher uma aparência para células que não são uma tabela as transforma em uma tabela. A aparência é desenhada por baixo do que a célula já tem de próprio: um preenchimento, bordas, uma cor de texto ou negrito que você definiu na célula continuam valendo.

Para criar uma aparência própria, escolha **Novo Estilo de Tabela** no final da galeria. Digite um nome. Depois escolha uma cor de preenchimento, uma cor de texto e negrito para cada parte que deseja alterar: a linha de cabeçalho, as faixas das linhas, a linha de totais e a primeira coluna. A nova aparência aparece em um grupo **Personalizado** no topo da galeria, e é aplicada à tabela em que você está. Ela é salva no arquivo, por isso continua disponível quando você abrir o arquivo novamente.

## A guia Tabela

A guia **Tabela** aparece enquanto o cursor está em uma tabela.

- **Nome** é o nome pelo qual as fórmulas chamam a tabela. Toda fórmula que usa o nome antigo passa a usar o novo. Um nome tem letras, dígitos, sublinhados e pontos, começa com uma letra ou um sublinhado, e não pode ser o nome de outra tabela, um nome da pasta de trabalho ou um endereço de célula, como A1.
- **Linha de Nomes**, **Linha de Totais**, **Linhas em Faixas**, **Colunas em Faixas**, **Primeira Coluna**, **Última Coluna** e **Botões de Filtro** são marcados ou desmarcados para a tabela.
- **Alterar Tamanho da Tabela…** pede as células que a tabela abrange. Ela mantém a primeira célula; linhas e colunas podem ser acrescentadas embaixo e à direita, ou removidas.
- **Resumir com Tabela Dinâmica** cria uma tabela dinâmica a partir das células da tabela (veja [Tabelas dinâmicas](pivot-tables)). A tabela dinâmica lê a tabela pelo nome, por isso acompanha a tabela quando ela cresce.
- **Remover Duplicatas…** elimina as linhas que se repetem. Uma linha se repete quando mostra o mesmo, em todas as colunas que você marcar, que uma linha acima dela. Letras maiúsculas e minúsculas são consideradas iguais. A primeira das linhas permanece, e as linhas abaixo de uma linha removida sobem. A tabela fica menor, e a linha de totais sobe junto. Um **Desfazer** devolve todas elas. As células abaixo da tabela continuam onde estão. Fórmulas que apontam para as células da tabela, em qualquer planilha, acompanham as células como acontece ao excluir linhas; uma fórmula que apontava para uma linha removida mostra #REF!. Não funciona em uma tabela com filtro ativado.
- **Voltar a Células Simples** transforma a tabela em células comuns. Elas mantêm o que contêm. Uma fórmula que citava a tabela ou suas colunas passa a citar as células.

## O menu das células

Clique com o botão direito em uma célula de uma tabela e escolha **Tabela**: **Inserir Linha Acima**, **Inserir Linha Abaixo**, **Inserir Coluna à Esquerda**, **Inserir Coluna à Direita**, **Excluir Linhas da Tabela**, **Excluir Colunas da Tabela**, **Linha de Totais** e **Voltar a Células Simples**. Eles inserem ou removem linhas e colunas inteiras da planilha, como Inserir e Excluir fazem; uma linha inserida logo após o fim da tabela é incorporada por ela.

## Linha de totais

Ative **Linha de Totais** e a linha abaixo da tabela tem a palavra Total na primeira coluna e um total na última. Cada célula da linha tem um botão: escolha o que o total da coluna mostra (a média, uma contagem, quantos números existem, o maior, o menor, a soma, o desvio padrão, a variância ou **Fórmula Personalizada**). Um total é um SUBTOTAL da sua coluna, por isso ignora as linhas que um filtro oculta.

## Acrescentar dados a uma tabela

- Digite na linha abaixo de uma tabela, ou na coluna ao lado dela, e a tabela cresce para incluir a célula. **Desfazer** remove o crescimento e mantém o que você digitou.
- Uma fórmula digitada em uma célula de uma coluna que está vazia é colocada em todas as células dessa coluna: uma coluna calculada. **Desfazer** logo em seguida mantém a fórmula apenas na célula. Uma fórmula digitada sobre uma coluna calculada altera a coluna inteira quando as demais células contêm a fórmula antiga; se algumas não a contiverem, a célula fica isolada.
- Linhas inseridas dentro de uma tabela recebem as fórmulas das colunas calculadas. Colunas inseridas dentro dela são nomeadas na linha de nomes. Excluir linhas ou colunas dentro dela a torna menor; a linha de nomes e todas as colunas não podem ser excluídas ao mesmo tempo.

## Filtrar e classificar uma tabela

Os botões dos nomes filtram e classificam apenas as linhas da tabela. Eles funcionam como os botões do filtro de uma planilha (veja [Classificar e filtrar](sort-filter)). Uma tabela não pode ser filtrada enquanto um filtro da planilha, ou de outra tabela, abranger as mesmas linhas, pois cada um precisaria saber quais linhas são suas.

## Fórmulas que citam uma tabela

- `Vendas[Valor]` são as células da coluna Valor, sem o nome dela e sem o total.
- `[@Valor]` é a célula da linha da própria fórmula, e é gravada como `Vendas[[#This Row],[Valor]]` no arquivo.
- `Vendas[#Headers]`, `Vendas[#Data]`, `Vendas[#Totals]` e `Vendas[#All]` são os nomes, os dados, os totais e a tabela inteira, e `Vendas[[#Headers],[Valor]]` é uma coluna deles.
- `Vendas[[Região]:[Valor]]` são as colunas de Região até Valor.

Digite `[` depois do nome de uma tabela e aparece uma lista das colunas dela. Enquanto você digita uma fórmula, clicar em uma célula de uma tabela escreve uma referência como `Vendas[@Valor]` ou `Vendas[Valor]` (clicar em uma coluna inteira de dados escreve a referência dessa coluna). Renomear uma coluna no cabeçalho a altera em todas as fórmulas; inserir e excluir colunas mantém as fórmulas apontadas para as colunas que citavam, e uma coluna excluída deixa #REF! nas fórmulas que a citavam.
