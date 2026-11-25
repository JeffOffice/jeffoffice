# Classificar e filtrar
<!-- keywords: classificar, filtrar, filtro automático, crescente, decrescente, A-Z, regras, cabeçalho, cor, lista, meses, dias da semana, maiúsculas, colunas, maiores, menores, média, entre, começa com, contém, data, limpar, reaplicar, lista suspensa, linhas ocultas, tabela, Ctrl+Shift+L -->

## Classificar

Selecione uma célula dos seus dados e escolha **Classificar de A a Z** ou **Classificar de Z a A** (**Classificar e Filtrar** na guia Página Inicial). O JeffCrunch usa todo o bloco de dados em volta da célula, mantém uma linha de cabeçalho quando encontra uma (texto sobre números) e põe as linhas em ordem pela coluna em que você está. Com várias células selecionadas, ele classifica apenas essas; se houver dados ao lado delas, pergunta se devem ser levados junto.

Na ordem, os números vêm primeiro, depois o texto, depois VERDADEIRO e FALSO, e por último os erros; células vazias sempre ficam por último. Letras maiúsculas e minúsculas são tratadas como iguais, a menos que você peça o contrário. O texto é ordenado do mesmo modo em todos os lugares: pelas letras do alfabeto inglês, com uma letra acentuada posicionada junto com a letra sem acento, qualquer que seja o idioma do computador.

Escolha **Classificar…** para abrir a caixa de diálogo Classificar:

- Adicione uma regra para cada coluna pela qual classificar. Uma regra posterior desempata as anteriores. Use **Mover para Cima** e **Mover para Baixo** para mudar a ordem das regras, e **Duplicar Regra** para começar a partir de uma delas.
- Classifique **Por** valores, pela **cor de preenchimento** das células ou pela **cor da fonte**. Para uma cor, escolha se ela vai **Primeiro** ou **Por último**; as demais linhas mantêm a ordem. Uma cor é a cor que ela aparenta: uma escolhida na paleta e uma com os mesmos valores de vermelho, verde e azul são iguais.
- Para valores, a ordem pode ser **A–Z**, **Z–A** ou uma lista: os dias da semana e os meses (por extenso e abreviados, em inglês e no seu idioma), ou uma lista que você digita, com vírgulas entre os valores. Valores que não estão na lista vão depois dos que estão.
- **A primeira linha contém os nomes das colunas** mantém a primeira linha fora da classificação. **Diferenciar maiúsculas** faz letras maiúsculas e minúsculas diferirem (minúsculas primeiro). **Classificar colunas, não linhas** põe as colunas em ordem pelos valores de uma linha. **Números em texto contam pelo valor** coloca "Item 2" antes de "Item 10" (sem essa opção, as letras e os dígitos são comparados um a um, e "Item 10" vem primeiro). A escolha vale para a classificação que você está fazendo, e a janela a lembra para a próxima classificação e para **Reaplicar**; o arquivo não a guarda, então **Reaplicar** depois de abrir o arquivo de novo classifica do modo simples, e a Verificação de Compatibilidade avisa sobre isso enquanto uma classificação mantida de um filtro ou de uma tabela tiver sido feita dessa forma (uma classificação simples posterior remove o aviso). Números de qualquer tamanho são comparados pelo valor.

Uma fórmula se move com a sua linha, e as referências a células da mesma linha continuam na linha. Uma referência a uma célula de outra linha é alterada como se a fórmula tivesse sido copiada. O que está ligado a uma célula vai junto com ela: a formatação, a nota e o link da célula se movem para o lugar em que a célula foi colocada. As regras que abrangem um intervalo de células (o que pode ser digitado e as formatações condicionais) permanecem onde estão, pois cobrem os lugares e não as células. A classificação é uma etapa que pode ser desfeita, e **Esc** interrompe uma classificação grande, que é feita aos poucos.

Algumas células não podem ser classificadas, e você é informado do motivo: células bloqueadas em uma planilha protegida, parte de uma matriz, células mescladas que não têm todas o mesmo tamanho (ou que ocupam mais de uma linha), células mescladas cortadas pela borda da seleção e a linha de cabeçalho de uma tabela.

## Filtrar

Selecione uma célula dos seus dados e escolha **Filtro** (**Ctrl+Shift+L**). Um botão aparece em cada célula da primeira linha. Clique em um deles para ver:

- **Classificar de A a Z** e **Classificar de Z a A**, e **Classificar por Cor**.
- Uma lista dos valores da coluna, com caixas de seleção, uma caixa de pesquisa e **(Tudo)**. Uma coluna de datas é listada por ano, mês e dia, e os valores dela que não são datas aparecem listados abaixo delas. Desmarque o que não quer e clique em **OK**. A pesquisa mostra os valores que contêm todas as palavras digitadas; **(Todos os resultados encontrados)** então marca ou desmarca apenas esses. A lista abrange a coluna inteira. Se a coluna tiver mais valores diferentes do que a lista comporta, o programa avisa e a lista é cortada; use uma condição para o restante.
- **Manter uma Cor**, pela cor de preenchimento ou pela cor da fonte.
- **Condições de Número**, **Condições de Texto** ou **Condições de Data**: é igual a, é maior que, é pelo menos, é menor que, é no máximo, está entre, os maiores ou menores (itens ou porcentagem), acima ou abaixo da média, começa com, termina com, contém, datas em um período (hoje, semana atual, próximo mês, trimestre anterior, até agora neste ano, um trimestre ou um mês de qualquer ano), e **Condição Personalizada** com duas comparações unidas por E ou Ou. No texto de uma condição, ? e \* representam qualquer caractere e qualquer sequência de caracteres, respectivamente.
- **Mostrar Tudo** da coluna.

Um botão com um funil indica uma coluna que tem filtro. As linhas que não correspondem ficam ocultas e mantêm seus números de linha. **Limpar** (no menu **Classificar e Filtrar**) remove todas as condições e mostra as linhas; **Reaplicar** recalcula as condições depois que você altera um valor. **Filtro** de novo desativa os botões.

As linhas ocultas por um filtro são separadas das linhas que você ocultou manualmente: **Reexibir Linhas** não mostra uma linha que o filtro oculta, e limpar o filtro não mostra uma linha que você ocultou. Classificar um bloco filtrado coloca em ordem apenas as linhas visíveis, entre si.

Um filtro atua sobre uma planilha inteira, com até 1.048.576 linhas, aos poucos; **Limpar** e desativar o filtro funcionam qualquer que seja o tamanho. O que um filtro precisa fazer é limitado a 40 milhões de verificações (as linhas vezes as colunas que têm condição, mais algumas para os valores listados nas condições); um filtro que exige mais avisa e não é feito, e então você pode filtrar menos linhas ou usar menos condições. Um filtro da planilha não abrange células de uma tabela: a tabela tem seus próprios botões, que funcionam do mesmo modo (veja [Tabelas](tables)).

Uma lista de valores em uma condição é guardada da forma como o arquivo a grava, sem idioma nos números e datas (1,234.50), e um valor corresponde a uma célula que o mostra no seu idioma ou nessa forma; um arquivo filtrado em um idioma filtra as mesmas células em outro.

O filtro é salvo no arquivo com as suas condições e as linhas que oculta, então o arquivo abre com as mesmas linhas visíveis. Ao reabrir um arquivo, o funil aparece em todas as colunas que têm condição; linhas ocultas no intervalo do filtro são contadas como do filtro.
