# Ordenar e filtrar
<!-- keywords: ordenar, filtrar, filtro automático, ascendente, descendente, A-Z, regras, cabeçalho, cor, lista, meses, dias da semana, maiúsculas, colunas, maior, menor, média, entre, começa por, contém, data, limpar, reaplicar, lista pendente, linhas ocultas, tabela, Ctrl+Shift+L -->

## Ordenar

Selecione uma célula dos seus dados e escolha **Ordenar de A a Z** ou **Ordenar de Z a A** (**Ordenar e Filtrar Linhas** no separador Base). O JeffCrunch utiliza todo o bloco de dados em redor da célula, mantém uma linha de cabeçalho sempre que a encontrar (texto sobre números) e coloca as linhas por ordem, segundo a coluna em que se encontra. Com várias células selecionadas, ordena apenas essas; se houver dados ao lado delas, pergunta se devem ser incluídos.

Na ordenação, os números vêm primeiro, depois o texto, depois VERDADEIRO e FALSO e, por fim, os erros; as células vazias ficam sempre no fim. As letras maiúsculas e minúsculas são tratadas como iguais, salvo indicação em contrário. O texto é ordenado da mesma forma em todo o lado: pelas letras do alfabeto inglês, com uma letra acentuada colocada junto da sua versão sem acento, seja qual for o idioma do computador.

Escolha **Ordenar…** para abrir a caixa de diálogo Ordenar:

- Adicione uma regra para cada coluna por que pretende ordenar. Uma regra posterior desempata as anteriores. Utilize **Mover para Cima** e **Mover para Baixo** para alterar a ordem das regras e **Duplicar Regra** para partir de uma já existente.
- Ordene **por** valores, pela **cor de preenchimento** das células ou pela **cor do texto**. Para uma cor, escolha se ela fica **Primeiro** ou **Último**; as restantes linhas mantêm a sua ordem. Uma cor é a cor que resulta: uma escolhida na paleta e outra com os mesmos valores de vermelho, verde e azul são iguais.
- Para valores, a ordem pode ser **A–Z**, **Z–A** ou uma lista: os dias da semana e os meses (por extenso e abreviados, em inglês e no seu idioma), ou uma lista que escreva, com vírgulas entre os valores. Os valores que não estão na lista ficam depois dos que estão.
- **A primeira linha contém os nomes das colunas** mantém a primeira linha fora da ordenação. **Diferenciar maiúsculas** faz com que as letras maiúsculas e minúsculas sejam diferentes (primeiro as minúsculas). **Ordenar colunas em vez de linhas** coloca as colunas por ordem, segundo os valores de uma linha. **Números em texto contam pelo valor** coloca "Item 2" antes de "Item 10" (sem esta opção, as letras e os algarismos são comparados um a um, e "Item 10" fica primeiro). A escolha vale para a ordenação que está a fazer, e a janela recorda-a para a ordenação seguinte e para **Reaplicar**; o ficheiro não a guarda. Por isso, **Reaplicar** depois de reabrir o ficheiro ordena de forma simples, e a Verificação de Compatibilidade indica-o enquanto uma ordenação guardada de um filtro ou de uma tabela tiver sido feita desta forma (uma ordenação simples posterior dessa tabela remove a indicação). Os números de qualquer comprimento são contados pelo valor.

Uma fórmula desloca-se com a sua linha, e as suas referências a células da mesma linha mantêm-se na linha. Uma referência a uma célula de outra linha é alterada como se a fórmula tivesse sido copiada. O que está ligado a uma célula acompanha-a: a formatação da célula, a sua nota e a sua ligação passam para o novo local. As regras que abrangem um intervalo de células (o que pode ser introduzido e a formatação condicional) mantêm-se onde estão, porque abrangem os locais e não as células. A ordenação é um passo que pode anular, e **Esc** interrompe uma ordenação grande; uma ordenação grande é feita aos poucos.

Algumas células não podem ser ordenadas, e é indicado o motivo: células bloqueadas numa folha protegida, parte de uma matriz, células unidas que não têm todas o mesmo tamanho (ou que abrangem mais de uma linha), células unidas cortadas pela extremidade da seleção e a linha de cabeçalho de uma tabela.

## Filtrar

Selecione uma célula dos seus dados e escolha **Filtro** (**Ctrl+Shift+L**). Aparece um botão em cada célula da primeira linha. Clique num deles para ver:

- **Ordenar de A a Z** e **Ordenar de Z a A**, e **Ordenar por Cor**.
- Uma lista dos valores da coluna, com caixas de verificação, uma caixa de pesquisa e **(Tudo)**. Uma coluna de datas é listada por ano, mês e dia, e os valores que não são datas aparecem listados abaixo destes. Desmarque o que não pretende e, depois, **OK**. A pesquisa mostra os valores que contêm todas as palavras escritas, e **(Tudo o que foi encontrado)** marca ou desmarca apenas esses. A lista abrange a coluna inteira. Se a coluna tiver mais valores diferentes do que a lista comporta, o programa indica-o e a lista fica truncada; utilize uma condição para o resto.
- **Manter uma Cor**, por cor de preenchimento ou cor do texto.
- **Condições de Números**, **Condições de Texto** ou **Condições de Datas**: é igual a, superior a, pelo menos, inferior a, no máximo, entre, os maiores ou menores (itens ou percentagem), acima ou abaixo da média, começa por, termina em, contém, datas num período (hoje, semana atual, mês seguinte, trimestre anterior, até à data deste ano, um trimestre ou um mês de qualquer ano), e **Condição Personalizada** com duas comparações ligadas por E ou Ou. No texto de uma condição, ? e \* representam qualquer carácter e qualquer sequência de caracteres.
- **Mostrar Tudo** da coluna.

Um botão com um funil indica uma coluna que tem um filtro. As linhas que não correspondem ficam ocultas e mantêm os seus números de linha. **Limpar** (no menu **Ordenar e Filtrar Linhas**) remove todas as condições e mostra as linhas; **Reaplicar** calcula novamente as condições depois de alterar um valor. **Filtro** novamente desativa os botões.

As linhas ocultas por um filtro são mantidas separadas das linhas que oculta manualmente: **Mostrar Linhas** não mostra uma linha que o filtro oculta, e limpar o filtro não mostra uma linha que tenha ocultado. Ordenar um bloco filtrado coloca apenas as linhas visíveis por ordem, entre si.

Um filtro atua sobre uma folha inteira, até 1 048 576 linhas, aos poucos; **Limpar** e desativar o filtro funcionam independentemente do tamanho. O que se pede a um filtro está limitado a 40 milhões de verificações (as linhas vezes as colunas que têm uma condição, com algumas para os valores listados nas condições); um filtro que pede mais do que isso indica-o e não é aplicado, podendo então filtrar menos linhas ou utilizar menos condições. Um filtro da folha não abrange as células de uma tabela: uma tabela tem os seus próprios botões, que funcionam da mesma forma (ver [Tabelas](tables)).

Uma lista de valores numa condição é guardada tal como um ficheiro a escreve, sem idioma nos números e nas datas (1,234.50), e um valor corresponde a uma célula que o mostra no seu idioma ou dessa forma; um ficheiro filtrado num idioma filtra as mesmas células noutro idioma.

O filtro é guardado no ficheiro com as suas condições e as linhas que oculta, por isso o ficheiro abre com as mesmas linhas visíveis. Ao reabrir um ficheiro, o funil aparece em todas as colunas que têm uma condição; as linhas ocultas no intervalo do filtro contam como do filtro.
