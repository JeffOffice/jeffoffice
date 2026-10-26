# Tabelas
<!-- keywords: tabela, tabelas, referência estruturada, linha de cabeçalho, linha de nomes, linha de totais, coluna calculada, linhas alternadas, listadas, estilo, aspetos da tabela, botões de filtro, Ctrl+T, converter, células simples, redimensionar, nome da coluna, [@Coluna], Tabela1[Coluna] -->

Uma tabela é um bloco de células com uma linha de nomes por cima das colunas, tratado como um só elemento: tem um nome, um aspeto, botões para filtrar e ordenar as linhas e aumenta quando lhe acrescenta dados. As fórmulas podem referir-se às colunas pelo nome em vez de pelo endereço das células.

## Criar uma tabela

Selecione as células, ou clique numa célula dos dados, e escolha **Tabela** no separador **Inserir** (**Ctrl+T**). O JeffCrunch pergunta quais as células abrangidas pela tabela e se a primeira linha contém os nomes das colunas. Os nomes que sejam números, datas ou fórmulas passam a ser o texto que mostram, os nomes vazios passam a Coluna1, Coluna2 e assim sucessivamente, e duas colunas com o mesmo nome são numeradas. Se não houver nomes, a tabela não tem linha de nomes e as colunas chamam-se Coluna1, Coluna2 e assim sucessivamente nas fórmulas.

A galeria **Aspetos da Tabela** (separador **Base** ou separador **Tabela**) mostra os aspetos disponíveis: claro, médio e escuro, cada um em preto e nas seis cores do tema do livro. Escolher um aspeto para células que não são uma tabela transforma-as numa tabela. O aspeto é desenhado por baixo da formatação própria de cada célula: um preenchimento, limites, uma cor de texto ou o negrito que definiu numa célula mantêm-se.

## O separador Tabela

O separador **Tabela** é apresentado enquanto o cursor estiver numa tabela.

- **Nome** é o nome pelo qual as fórmulas se referem à tabela. Todas as fórmulas que utilizam o nome antigo são atualizadas. Um nome tem letras, algarismos, carateres de sublinhado e pontos, começa por uma letra ou por um carácter de sublinhado e não pode ser o nome de outra tabela, um nome do livro nem um endereço de célula como A1.
- **Linha de Nomes**, **Linha de Totais**, **Linhas Listadas**, **Colunas Listadas**, **Primeira Coluna**, **Última Coluna** e **Botões de Filtro** são ativados ou desativados para a tabela.
- **Alterar Tamanho da Tabela…** pede as células abrangidas pela tabela. A primeira célula mantém-se; é possível acrescentar ou retirar linhas e colunas em baixo e à direita.
- **Converter em Células Simples** transforma a tabela em células normais, que mantêm o conteúdo. Uma fórmula que se referia à tabela ou às respetivas colunas passa a referir-se às células.

## O menu das células

Clique com o botão direito do rato numa célula de uma tabela e escolha **Tabela**: **Inserir Linha Acima**, **Inserir Linha Abaixo**, **Inserir Coluna à Esquerda**, **Inserir Coluna à Direita**, **Eliminar Linhas da Tabela**, **Eliminar Colunas da Tabela**, **Linha de Totais** e **Converter em Células Simples**. Estes comandos inserem ou eliminam linhas e colunas inteiras da folha, tal como Inserir e Eliminar; uma linha ou coluna inserida imediatamente a seguir ao fim da tabela passa a fazer parte dela.

## Linha de totais

Ative **Linha de Totais** e a linha abaixo da tabela mostra a palavra Total na primeira coluna e um total na última. Cada célula dessa linha tem um botão: escolha o que o total da coluna mostra (a média, uma contagem, quantos números existem, o maior valor, o menor valor, a soma, o desvio padrão, a variância ou **Fórmula Personalizada…**). Um total é um SUBTOTAL da respetiva coluna, pelo que exclui as linhas ocultadas por um filtro.

## Acrescentar dados a uma tabela

- Escreva na linha abaixo de uma tabela, ou na coluna ao lado, e a tabela aumenta para incluir a célula. **Anular** desfaz o aumento e mantém o que escreveu.
- Uma fórmula escrita numa célula de uma coluna que, de resto, está vazia é inserida em todas as células dessa coluna: é uma coluna calculada. Se utilizar **Anular** imediatamente a seguir, a fórmula fica apenas nessa célula. Uma fórmula escrita sobre uma coluna calculada altera a coluna inteira quando as outras células contêm a fórmula antiga; se algumas não a contiverem, a célula fica isolada.
- As linhas inseridas dentro de uma tabela recebem as fórmulas das colunas calculadas. As colunas inseridas dentro dela recebem um nome na linha de nomes. Eliminar linhas ou colunas dentro da tabela torna-a mais pequena; não é possível eliminar ao mesmo tempo a linha de nomes e todas as colunas.

## Filtrar e ordenar uma tabela

Os botões dos nomes filtram e ordenam apenas as linhas da tabela. Funcionam como os botões do filtro de uma folha (consulte [Ordenar e filtrar](sort-filter)). Não é possível filtrar uma tabela enquanto um filtro da folha, ou de outra tabela, abranger as mesmas linhas, pois cada um precisaria de saber quais as linhas que lhe pertencem.

## Fórmulas que se referem a uma tabela

- `Sales[Amount]` são as células da coluna Amount, sem o nome e o total.
- `[@Amount]` é a célula da própria linha da fórmula e é escrita `Sales[[#This Row],[Amount]]` no ficheiro.
- `Sales[#Headers]`, `Sales[#Data]`, `Sales[#Totals]` e `Sales[#All]` são os nomes, os dados, os totais e a tabela inteira, e `Sales[[#Headers],[Amount]]` é uma coluna desses elementos.
- `Sales[[Region]:[Amount]]` são as colunas de Region a Amount.

Escreva `[` a seguir ao nome de uma tabela e aparece uma lista das respetivas colunas. Enquanto escreve uma fórmula, clicar numa célula de uma tabela escreve uma referência como `Sales[@Amount]` ou `Sales[Amount]` (ao clicar numa coluna inteira dos dados). Mudar o nome de uma coluna no cabeçalho altera-o em todas as fórmulas; inserir e eliminar colunas mantém as fórmulas a apontar para as colunas que referiam, e uma coluna eliminada deixa #REF! nas fórmulas que se referiam a ela.
