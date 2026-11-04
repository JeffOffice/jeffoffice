# Tabelas
<!-- keywords: tabela, tabelas, remover duplicatas, duplicatas, resumir, limpar estilo, referência estruturada, linha de cabeçalho, linha de total, coluna calculada, linhas alternadas, faixas, estilo, aparência da tabela, botões de filtro, Ctrl+T, converter, células simples, redimensionar, nome da coluna, [@Coluna], Tabela1[Coluna] -->

Uma tabela é um bloco de células com uma linha de nomes sobre as colunas, tratado como um único elemento: tem um nome, uma aparência, botões para filtrar e classificar as suas linhas, e cresce quando você acrescenta dados. As fórmulas podem chamar as colunas dela pelo nome, em vez de pelo endereço das células.

## Criar uma tabela

Selecione as células, ou clique em uma célula dos dados, e escolha **Tabela** na guia **Inserir** (**Ctrl+T**). O JeffCrunch pergunta quais células a tabela abrange e se a primeira linha contém os nomes das colunas. Nomes que são números, datas ou fórmulas passam a ser o texto que eles exibem, nomes vazios se tornam Coluna1, Coluna2, e assim por diante, e duas colunas com o mesmo nome são numeradas. Se não houver nomes, a tabela não terá linha de nomes, e as suas colunas se chamarão Coluna1, Coluna2, e assim por diante, nas fórmulas.

A galeria **Aparência da Tabela** (guia **Página Inicial** ou guia **Tabela**) mostra as aparências (**Limpar**, abaixo da galeria na guia **Tabela**, remove a aparência da tabela): clara, média e escura, cada uma em preto e nas seis cores do tema da pasta de trabalho. Escolher uma aparência para células que não são uma tabela as transforma em uma tabela. A aparência é desenhada por baixo do que a célula tem de próprio: um preenchimento, bordas, uma cor de texto ou negrito que você definiu na célula continua valendo.

## A guia Tabela

A guia **Tabela** aparece enquanto o cursor está em uma tabela.

- **Nome** é o nome pelo qual as fórmulas chamam a tabela. Toda fórmula que usa o nome antigo passa a usar o novo. Um nome tem letras, dígitos, sublinhados e pontos, começa com uma letra ou um sublinhado, e não pode ser o nome de outra tabela, um nome da pasta de trabalho nem um endereço de célula, como A1.
- **Linha de Nomes**, **Linha de Total**, **Linhas em Faixas**, **Colunas em Faixas**, **Primeira Coluna**, **Última Coluna** e **Botões de Filtro** são marcados ou desmarcados para a tabela.
- **Alterar Tamanho da Tabela…** pede as células que a tabela abrange. Ela mantém a primeira célula; linhas e colunas podem ser acrescentadas na parte inferior e à direita, ou removidas.
- **Resumir com Tabela Dinâmica** cria uma tabela dinâmica a partir das células da tabela (veja [Tabelas dinâmicas](pivot-tables)). A tabela dinâmica lê a tabela pelo nome, por isso acompanha a tabela quando ela cresce.
- **Remover Duplicatas…** elimina as linhas que se repetem. Uma linha se repete quando mostra o mesmo, em todas as colunas que você marcar, que uma linha acima dela. Letras maiúsculas e minúsculas são consideradas iguais. A primeira das linhas permanece, e as linhas abaixo de uma linha removida sobem. A tabela fica mais curta, e a sua linha de total sobe junto. Um **Desfazer** devolve todas elas. As células abaixo da tabela permanecem onde estão. Fórmulas que apontam para células da tabela, em qualquer planilha, acompanham as células como acontece ao excluir linhas; uma fórmula que apontava para uma linha removida mostra #REF!. Não funciona em uma tabela com filtro ativo.
- **Voltar a Células Simples** transforma a tabela em células comuns. Elas mantêm o que contêm. Uma fórmula que citava a tabela ou as suas colunas passa a citar as células.

## O menu das células

Clique com o botão direito em uma célula da tabela e escolha **Tabela**: **Inserir Linha Acima**, **Inserir Linha Abaixo**, **Inserir Coluna à Esquerda**, **Inserir Coluna à Direita**, **Excluir Linhas da Tabela**, **Excluir Colunas da Tabela**, **Linha de Total** e **Voltar a Células Simples**. Eles inserem ou removem linhas e colunas inteiras da planilha, como Inserir e Excluir fazem; uma linha inserida logo após o fim da tabela é incorporada por ela.

## Linha de total

Ative **Linha de Total** e a linha abaixo da tabela mostra a palavra Total na sua primeira coluna e um total na última. Cada célula da linha tem um botão: escolha o que o total da coluna mostra (a média, uma contagem, quantos números há, o maior, o menor, a soma, o desvio padrão, a variância ou **Fórmula Personalizada**). Um total é um SUBTOTAL da sua coluna, por isso ignora as linhas que um filtro oculta.

## Acrescentar dados a uma tabela

- Digite na linha abaixo de uma tabela, ou na coluna ao lado dela, e a tabela cresce para incluir a célula. **Desfazer** desfaz o crescimento e mantém o que você digitou.
- Uma fórmula digitada em uma célula de uma coluna que está vazia no restante é colocada em todas as células dessa coluna: uma coluna calculada. **Desfazer** logo em seguida deixa a fórmula apenas na célula. Uma fórmula digitada sobre uma coluna calculada altera a coluna inteira quando as outras células contêm a fórmula antiga; se algumas não a contiverem, a célula fica isolada.
- Linhas inseridas dentro de uma tabela recebem as fórmulas das colunas calculadas. Colunas inseridas dentro dela são nomeadas na linha de nomes. Excluir linhas ou colunas dentro dela a torna menor; a linha de nomes e todas as colunas não podem ser excluídas ao mesmo tempo.

## Filtrar e classificar uma tabela

Os botões dos nomes filtram e classificam apenas as linhas da tabela. Eles funcionam como os botões do filtro de uma planilha (veja [Classificar e filtrar](sort-filter)). Uma tabela não pode ser filtrada enquanto um filtro da planilha, ou de outra tabela, abranger as mesmas linhas, pois cada um precisaria saber quais linhas são suas.

## Fórmulas que citam uma tabela

- `Vendas[Valor]` são as células da coluna Valor, sem o nome e o total dela.
- `[@Valor]` é a célula da linha da própria fórmula, e é escrita `Vendas[[#Esta Linha],[Valor]]` no arquivo.
- `Vendas[#Cabeçalhos]`, `Vendas[#Dados]`, `Vendas[#Totais]` e `Vendas[#Tudo]` são os nomes, os dados, os totais e a tabela inteira, e `Vendas[[#Cabeçalhos],[Valor]]` é uma coluna deles.
- `Vendas[[Região]:[Valor]]` são as colunas de Região até Valor.

Digite `[` depois do nome de uma tabela e aparece uma lista das suas colunas. Enquanto você digita uma fórmula, clicar em uma célula da tabela escreve uma referência como `Vendas[@Valor]` ou `Vendas[Valor]` (clicar em toda a coluna de dados). Renomear uma coluna no cabeçalho a altera em todas as fórmulas; inserir e excluir colunas mantém as fórmulas apontando para as colunas que citavam, e uma coluna excluída deixa #REF! nas fórmulas que a citavam.
