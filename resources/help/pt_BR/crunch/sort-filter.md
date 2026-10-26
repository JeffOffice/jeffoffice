# Classificar e filtrar
<!-- keywords: classificar, ordenar, ordem, filtrar, filtro, autofiltro, crescente, decrescente, A-Z, regras, cabeçalho, cor, lista, meses, dias da semana, maiúsculas, colunas, maiores, menores, média, entre, começa com, contém, data, limpar, reaplicar, lista suspensa, linhas ocultas, tabela, Ctrl+Shift+L -->

## Classificar

Selecione uma célula dos seus dados e escolha **Classificar de A a Z** ou **Classificar de Z a A** (**Classificar e Filtrar** na guia Página Inicial). O JeffCrunch usa todo o bloco de dados ao redor da célula, mantém uma linha de cabeçalho quando encontra uma (texto sobre números) e coloca as linhas em ordem pela coluna em que você está. Com várias células selecionadas, ele classifica somente essas células; se houver dados ao lado delas, ele pergunta se deve incluí-los.

Na ordem, os números vêm primeiro, depois o texto, depois VERDADEIRO e FALSO e, por último, os erros; as células vazias sempre ficam no final. Letras maiúsculas e minúsculas são consideradas iguais, a menos que você peça o contrário. O texto é ordenado da mesma forma em qualquer lugar: pelas letras do inglês, com cada letra acentuada junto da letra sem acento, seja qual for o idioma do computador.

Escolha **Classificar…** para abrir a caixa de diálogo Classificar:

- Adicione uma regra para cada coluna pela qual classificar. Uma regra posterior desempata as anteriores. Use **Mover para Cima** e **Mover para Baixo** para alterar a ordem das regras e **Duplicar Regra** para começar a partir de uma regra existente.
- Em **Classificar em**, escolha os valores, a **cor de preenchimento** das células ou a **cor do texto**. Para uma cor, escolha se ela fica em **Primeiro** ou em **Último**; as outras linhas mantêm a ordem. Uma cor vale pelo resultado final: uma cor escolhida na paleta e outra com os mesmos valores de vermelho, verde e azul são a mesma cor.
- Para valores, a ordem pode ser **A–Z**, **Z–A** ou uma lista: os dias da semana e os meses (completos e abreviados, em inglês e no seu idioma) ou uma lista que você digita, com vírgulas entre os valores. Os valores que não estão na lista ficam depois dos que estão.
- **A primeira linha contém os nomes das colunas** deixa a primeira linha fora da classificação. **Diferenciar maiúsculas de minúsculas** faz as letras maiúsculas e minúsculas serem diferentes (minúsculas primeiro). **Classificar colunas, não linhas** coloca as colunas em ordem pelos valores de uma linha.

Uma fórmula se move com a linha dela, e as referências dela a células da mesma linha continuam na linha. Uma referência a uma célula de outra linha é alterada como se a fórmula tivesse sido copiada. O que está vinculado a uma célula vai com ela: o formato da célula, a anotação e o link vão para o local para onde a célula vai. As regras que abrangem um intervalo de células (o que pode ser digitado e as formatações condicionais) ficam onde estão, pois abrangem os locais, e não as células. A classificação é uma única etapa que pode ser desfeita, e **Esc** interrompe uma classificação grande; uma classificação grande é aplicada aos poucos.

Algumas células não podem ser classificadas, e o motivo é informado: células bloqueadas em uma planilha protegida, partes de uma matriz, células mescladas que não têm todas o mesmo tamanho (ou que abrangem mais de uma linha), células mescladas cortadas pela borda da seleção e a linha de cabeçalho de uma tabela.

## Filtrar

Selecione uma célula dos seus dados e escolha **Filtro** (**Ctrl+Shift+L**). Aparece um botão em cada célula da primeira linha. Clique em um deles para ver:

- **Classificar de A a Z**, **Classificar de Z a A** e **Classificar por cor**.
- Uma lista dos valores da coluna com caixas de seleção, uma caixa de pesquisa e **(Tudo)**. Uma coluna de datas é listada por ano, mês e dia, e os valores dela que não são datas aparecem abaixo. Desmarque o que você não quer e clique em **OK**. A pesquisa mostra os valores que contêm todas as suas palavras; **(Todos os resultados encontrados)** marca ou desmarca somente esses valores. A lista abrange a coluna inteira. Se a coluna tiver mais valores diferentes do que a lista comporta, isso é informado e a lista fica incompleta; use uma condição para o restante.
- **Manter uma cor**, por cor de preenchimento ou cor do texto.
- **Condições de número**, **Condições de texto** ou **Condições de data**: é igual a, maior que, maior ou igual a, menor que, menor ou igual a, entre, os maiores ou menores (itens ou porcentagem), acima ou abaixo da média, começa com, termina com, contém, datas em um período (hoje, esta semana, próximo mês, trimestre passado, até agora neste ano, um trimestre ou um mês de qualquer ano) e **Condição personalizada**, com duas comparações unidas por E ou Ou. No texto de uma condição, ? e \* representam qualquer caractere e qualquer sequência de caracteres.
- **Mostrar tudo** da coluna.

Um botão com um funil indica uma coluna que tem filtro. As linhas que não correspondem ficam ocultas e mantêm os números de linha. **Limpar** (no menu **Classificar e Filtrar**) remove todas as condições e mostra as linhas; **Reaplicar** calcula as condições novamente depois que você altera um valor. Escolher **Filtro** de novo desativa os botões.

As linhas ocultas por um filtro são tratadas separadamente das linhas que você mesmo ocultou: **Reexibir Linhas** não mostra uma linha que o filtro oculta, e limpar o filtro não mostra uma linha que você ocultou. Classificar um bloco filtrado coloca em ordem somente as linhas visíveis, entre si.

Um filtro funciona em uma planilha inteira, com até 1.048.576 linhas, aos poucos; **Limpar** e desativar o filtro funcionam seja qual for o tamanho. O trabalho de um filtro é limitado a 40 milhões de verificações (as linhas vezes as colunas que têm condição, mais algumas para os valores que as condições listam); um filtro que exige mais avisa e não é criado, e você pode filtrar menos linhas ou usar menos condições. Um filtro da planilha não abrange as células de uma tabela: uma tabela tem os próprios botões, que funcionam da mesma forma (consulte [Tabelas](tables)).

Uma lista de valores em uma condição é guardada como o arquivo a grava, sem idioma nos números e datas (1,234.50), e um valor corresponde a uma célula que o mostra no seu idioma ou dessa forma; um arquivo filtrado em um idioma filtra as mesmas células em outro.

O filtro é salvo no arquivo com as condições e as linhas que ele oculta, de modo que o arquivo abre com as mesmas linhas visíveis. Ao reabrir um arquivo, o funil aparece em todas as colunas que têm condição; as linhas ocultas no intervalo do filtro são consideradas do filtro.
