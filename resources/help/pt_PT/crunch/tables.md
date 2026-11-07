# Tabelas
<!-- keywords: tabela, tabelas, remover duplicados, duplicados, resumir, limpar estilo, referência estruturada, linha de cabeçalho, linha de totais, coluna calculada, listada, alternada, estilo, aspetos da tabela, novo estilo de tabela, estilo de tabela personalizado, botões de filtro, Ctrl+T, converter, células simples, redimensionar, nome de coluna, [@Coluna], Tabela1[Coluna] -->

Uma tabela é um bloco de células com uma linha de nomes sobre as suas colunas, tratado como um único elemento: tem um nome, um aspeto, botões para filtrar e ordenar as suas linhas, e cresce quando lhe acrescenta dados. As fórmulas podem referir as colunas da tabela pelo nome, em vez de pelo endereço das células.

## Criar uma tabela

Selecione as células, ou clique numa célula dos dados, e escolha **Tabela** no separador **Inserir** (**Ctrl+T**). O JeffCrunch pergunta que células a tabela abrange e se a primeira linha contém os nomes das colunas. Os nomes que são números, datas ou fórmulas passam a ser o texto que apresentam, os nomes vazios passam a ser Coluna1, Coluna2, e assim por diante, e duas colunas com o mesmo nome são numeradas. Se não houver nomes, a tabela não tem linha de nomes, e as suas colunas chamam-se Coluna1, Coluna2, e assim por diante, nas fórmulas.

A galeria **Aspetos da Tabela** (separador **Base**, ou separador **Tabela**) mostra os aspetos disponíveis: claro, médio e escuro, cada um em preto e nas seis cores do tema do livro. **Limpar**, por baixo da galeria no separador **Tabela**, retira o aspeto à tabela. Se escolher um aspeto para células que não são uma tabela, estas passam a ser uma tabela. O aspeto é desenhado por baixo do que a célula tem de próprio: um preenchimento, limites, uma cor de texto ou negrito que tenha definido numa célula mantém-se.

Para criar um aspeto próprio, escolha **Novo Estilo de Tabela** no fundo da galeria. Escreva um nome. Depois escolha uma cor de preenchimento, uma cor de texto e negrito para cada parte que queira alterar: a linha de cabeçalho, as linhas listadas, a linha de totais e a primeira coluna. O novo aspeto aparece num grupo **Personalizado**, no topo da galeria, e é aplicado à tabela em que está. Fica guardado no ficheiro, por isso continua disponível quando o abrir novamente.

## O separador Tabela

O separador **Tabela** aparece enquanto o cursor estiver numa tabela.

- **Nome** é o nome pelo qual as fórmulas se referem à tabela. Todas as fórmulas que usam o nome antigo passam a usar o novo. Um nome tem letras, algarismos, sublinhados e pontos, começa por uma letra ou por um sublinhado, e não pode ser o de outra tabela, o de um nome do livro, nem um endereço de célula como A1.
- **Linha de Nomes**, **Linha de Totais**, **Linhas Listadas**, **Colunas Listadas**, **Primeira Coluna**, **Última Coluna** e **Botões de Filtro** são ativados ou desativados para a tabela.
- **Alterar Tamanho da Tabela…** pede as células que a tabela abrange. Mantém a primeira célula; podem ser acrescentadas linhas e colunas no fim e à direita, ou retiradas.
- **Resumir com Tabela Dinâmica** cria uma tabela dinâmica a partir das células da tabela (veja [Tabelas dinâmicas](pivot-tables)). A tabela dinâmica lê a tabela pelo nome, por isso acompanha-a quando ela cresce.
- **Remover Duplicados…** elimina as linhas que se repetem. Uma linha repete-se quando mostra o mesmo, em todas as colunas que marcar, que uma linha acima dela. As maiúsculas e as minúsculas são consideradas iguais. Fica a primeira das linhas, e as linhas abaixo de uma linha eliminada sobem. A tabela fica mais curta, e a linha de totais sobe com ela. Um **Anular** repõe tudo. As células por baixo da tabela mantêm-se onde estão. As fórmulas que apontam para as células da tabela, em qualquer folha, acompanham as células como acontece quando elimina linhas; uma fórmula que apontava para uma linha eliminada mostra #REF!. Esta opção não funciona numa tabela com um filtro ativo.
- **Converter em Células Simples** transforma a tabela em células normais. Estas mantêm o que contêm. Uma fórmula que indicava a tabela ou as suas colunas passa a indicar as células.

## O menu das células

Clique com o botão direito numa célula de uma tabela e escolha **Tabela**: **Inserir Linha Acima**, **Inserir Linha Abaixo**, **Inserir Coluna à Esquerda**, **Inserir Coluna à Direita**, **Eliminar Linhas da Tabela**, **Eliminar Colunas da Tabela**, **Linha de Totais** e **Converter em Células Simples**. Estes comandos acrescentam ou retiram linhas e colunas inteiras da folha, como fazem Inserir e Eliminar; uma linha acrescentada logo a seguir ao fim da tabela é incorporada pela tabela.

## Linha de totais

Ative **Linha de Totais** e a linha por baixo da tabela passa a ter a palavra Total na primeira coluna e um total na última. Cada célula da linha tem um botão: escolha o que mostra o total da coluna (a média, uma contagem, quantos números existem, o maior, o menor, a soma, o desvio padrão, a variância ou **Fórmula Personalizada…**). Um total é um SUBTOTAL da sua coluna, por isso ignora as linhas que um filtro oculta.

## Acrescentar dados a uma tabela

- Escreva na linha abaixo de uma tabela, ou na coluna ao lado, e a tabela cresce para incluir a célula. **Anular** desfaz o crescimento e mantém o que escreveu.
- Uma fórmula escrita numa célula de uma coluna que está vazia é colocada em todas as células dessa coluna: uma coluna calculada. Se carregar em **Anular** logo a seguir, a fórmula fica apenas na célula. Uma fórmula escrita sobre uma coluna calculada altera a coluna inteira quando as outras células têm a fórmula antiga; se algumas não a tiverem, a célula fica isolada.
- As linhas inseridas dentro de uma tabela recebem as fórmulas das colunas calculadas. As colunas inseridas dentro dela são nomeadas na linha de nomes. Eliminar linhas ou colunas dentro da tabela torna-a mais pequena; a linha de nomes e todas as colunas não podem ser eliminadas em simultâneo.

## Filtrar e ordenar uma tabela

Os botões nos nomes filtram e ordenam apenas as linhas da tabela. Funcionam como os botões do filtro de uma folha (veja [Ordenar e Filtrar](sort-filter)). Não é possível filtrar uma tabela enquanto um filtro da folha, ou de outra tabela, abranger as mesmas linhas, pois cada um precisaria de saber quais as linhas que lhe pertencem.

## Fórmulas que indicam uma tabela

- `Vendas[Montante]` são as células da coluna Montante, sem o nome nem o total.
- `[@Montante]` é a célula da linha da própria fórmula, e é escrita `Vendas[[#This Row],[Montante]]` no ficheiro.
- `Vendas[#Headers]`, `Vendas[#Data]`, `Vendas[#Totals]` e `Vendas[#All]` são os nomes, os dados, os totais e a tabela inteira, e `Vendas[[#Headers],[Montante]]` é uma coluna deles.
- `Vendas[[Região]:[Montante]]` são as colunas de Região até Montante.

Escreva `[` depois do nome de uma tabela e aparece uma lista das suas colunas. Enquanto escreve uma fórmula, clicar numa célula de uma tabela escreve uma referência como `Vendas[@Montante]` ou `Vendas[Montante]` (clicar numa coluna inteira de dados escreve a coluna completa). Mudar o nome de uma coluna no respetivo cabeçalho altera-o em todas as fórmulas; inserir e eliminar colunas mantém as fórmulas apontadas para as colunas a que se referiam, e uma coluna eliminada deixa #REF! nas fórmulas que a indicavam.
