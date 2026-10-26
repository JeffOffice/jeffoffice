# Ordenar e filtrar
<!-- keywords: ordenar, ordenação, filtrar, filtro, filtro automático, ascendente, descendente, crescente, decrescente, A-Z, regras, cabeçalho, cor, lista, meses, dias da semana, maiúsculas, colunas, maiores, menores, média, entre, começa por, contém, data, limpar, reaplicar, lista pendente, linhas ocultas, tabela, Ctrl+Shift+L -->

## Ordenar

Selecione uma célula dos dados e escolha **Ordenar de A a Z** ou **Ordenar de Z a A** (**Ordenar e Filtrar Linhas** no separador **Base**). O JeffCrunch utiliza todo o bloco de dados à volta da célula, mantém uma linha de cabeçalho quando a encontra (texto por cima de números) e ordena as linhas pela coluna onde se encontra. Com várias células selecionadas, ordena apenas essas; se existirem dados ao lado, pergunta se os deve incluir.

Na ordenação, os números vêm primeiro, depois o texto, depois VERDADEIRO e FALSO e, por fim, os erros; as células vazias ficam sempre em último lugar. As maiúsculas e as minúsculas são tratadas como iguais, a menos que indique o contrário. O texto é ordenado da mesma forma em todo o lado: pelas letras do inglês, com cada letra acentuada colocada junto da letra sem acento, seja qual for o idioma do computador.

Escolha **Ordenar…** para abrir a caixa de diálogo **Ordenar**:

- Adicione uma regra para cada coluna pela qual pretende ordenar. Uma regra posterior desempata as anteriores. Utilize **Mover para Cima** e **Mover para Baixo** para alterar a ordem das regras e **Duplicar Regra** para partir de uma existente.
- Ordene **Utilizando** os valores, a **cor de preenchimento** das células ou a **cor do texto**. Para uma cor, escolha se fica em **Primeiro** ou em **Último**; as restantes linhas mantêm a ordem. Uma cor é a cor que resulta: uma cor escolhida na paleta e outra com os mesmos valores de vermelho, verde e azul são iguais.
- Para valores, a ordem pode ser **A–Z**, **Z–A** ou uma lista: os dias da semana e os meses (por extenso e abreviados, em inglês e no seu idioma) ou uma lista que escreva, com vírgulas entre os valores. Os valores que não constam da lista ficam depois dos que constam.
- **A primeira linha contém os nomes das colunas** exclui a primeira linha da ordenação. **Diferenciar maiúsculas** distingue maiúsculas de minúsculas (minúsculas primeiro). **Ordenar colunas em vez de linhas** ordena as colunas pelos valores de uma linha.

Uma fórmula move-se com a respetiva linha, e as referências a células da mesma linha mantêm-se nessa linha. Uma referência a uma célula noutra linha é alterada como se a fórmula fosse copiada. O que está associado a uma célula acompanha-a: o formato, a nota e a ligação da célula passam para o local para onde a célula vai. As regras que abrangem um intervalo de células (o que pode ser introduzido e a formatação condicional) ficam onde estão, porque abrangem os locais e não as células. A ordenação é um único passo que pode anular, e **Esc** interrompe uma ordenação grande; uma ordenação grande é aplicada pouco a pouco.

Algumas células não podem ser ordenadas, e é-lhe indicado o motivo: células bloqueadas numa folha protegida, parte de uma matriz, células unidas que não têm todas o mesmo tamanho (ou que abrangem mais do que uma linha), células unidas cortadas pelo limite da seleção e a linha de cabeçalho de uma tabela.

## Filtrar

Selecione uma célula dos dados e escolha **Filtrar** (**Ctrl+Shift+L**). Aparece um botão em cada célula da primeira linha. Clique num deles para ver:

- **Ordenar de A a Z** e **Ordenar de Z a A**, e **Ordenar por cor**.
- Uma lista dos valores da coluna com caixas de verificação, uma caixa de pesquisa e **(Tudo)**. Uma coluna de datas é apresentada por ano, mês e dia, e os valores que não são datas aparecem por baixo. Desmarque o que não pretende e clique em **OK**. A pesquisa mostra os valores que contêm todas as palavras escritas; **(Tudo o que foi encontrado)** marca ou desmarca apenas esses. A lista abrange a coluna inteira. Se a coluna tiver mais valores diferentes do que a lista comporta, isso é indicado e a lista fica incompleta; utilize uma condição para os restantes.
- **Manter uma cor**, pela cor de preenchimento ou pela cor do texto.
- **Condições de números**, **Condições de texto** ou **Condições de datas**: é igual a, é maior que, é maior ou igual a, é menor que, é menor ou igual a, entre, os maiores ou menores (itens ou percentagem), acima ou abaixo da média, começa por, termina em, contém, datas num período (hoje, esta semana, próximo mês, trimestre passado, ano até à data, um trimestre ou um mês de qualquer ano) e **Condição Personalizada** com duas comparações ligadas por E ou Ou. No texto de uma condição, ? e \* representam qualquer carácter individual e qualquer sequência de carateres.
- **Show All of** (mostrar tudo da coluna).

Um botão com um funil indica uma coluna com filtro. As linhas que não correspondem ficam ocultas e mantêm os números de linha. **Limpar** (no menu **Ordenar e Filtrar Linhas**) remove todas as condições e mostra as linhas; **Reaplicar** volta a avaliar as condições depois de alterar um valor. Escolher novamente **Filtrar** desativa os botões.

As linhas ocultas por um filtro são tratadas separadamente das linhas que ocultou manualmente: **Mostrar Linhas** não mostra uma linha que o filtro oculta, e limpar o filtro não mostra uma linha que ocultou. Ordenar um bloco filtrado ordena apenas as linhas visíveis, entre si.

Um filtro funciona numa folha inteira, até 1 048 576 linhas, pouco a pouco; **Limpar** e desativar o filtro funcionam seja qual for o tamanho. O trabalho pedido a um filtro está limitado a 40 milhões de verificações (as linhas vezes as colunas que têm condição, mais algumas pelos valores indicados nas condições); um filtro que exija mais indica-o e não é criado, podendo filtrar menos linhas ou utilizar menos condições. Um filtro da folha não abrange células de uma tabela: uma tabela tem os seus próprios botões, que funcionam da mesma forma (consulte [Tabelas](tables)).

Uma lista de valores numa condição é guardada tal como um ficheiro a escreve, sem idioma nos números e datas (1,234.50), e um valor corresponde a uma célula que o mostre no seu idioma ou dessa forma; um ficheiro filtrado num idioma filtra as mesmas células noutro.

O filtro é guardado no ficheiro com as condições e as linhas que oculta, pelo que o ficheiro abre com as mesmas linhas visíveis. Ao reabrir um ficheiro, o funil aparece em todas as colunas que têm condição; as linhas ocultas no intervalo do filtro são consideradas como ocultas pelo filtro.
