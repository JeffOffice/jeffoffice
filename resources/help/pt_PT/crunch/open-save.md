# Abrir e guardar livros
<!-- keywords: abrir, guardar, guardar como, gravar, ficheiro, livro, folha de cálculo, xlsx, xlsm, xls, ods, csv, tsv, ficheiro de texto, delimitado, separado por vírgulas, importar, exportar, recentes, afixados, arrastar, largar, carregar, carregamento, grande, macros, não guardado, alterações, fechar, cópia, cadeia de cálculo -->

Para abrir um livro, prima **{key:Open}** ou clique em **Ficheiro**, em **Abrir** e escolha **Procurar**. Também pode arrastar um livro do gestor de ficheiros para a janela. Um livro que já esteja aberto numa janela não é aberto uma segunda vez: essa janela passa para a frente.

O JeffCrunch abre livros **.xlsx** e livros **.xlsm** (os que contêm macros). Não executa as macros e mantém-nas intactas.

## Outros tipos de ficheiros

O JeffCrunch também abre livros **.xls** mais antigos, folhas de cálculo OpenDocument (**.ods** e o formato simples **.fods**) e ficheiros de texto de valores (**.csv**, **.tsv** e **.txt**). Um livro aberto a partir de um destes ficheiros é guardado como um novo ficheiro **.xlsx**: quando prime **{key:Save}**, o JeffCrunch informa-o disso e pede um nome, e o ficheiro que abriu nunca é alterado.

Uma folha de cálculo OpenDocument é lida com as respetivas folhas, valores, fórmulas (com os resultados que o ficheiro guardou), formatos numéricos, tipos de letra, preenchimentos, limites, células unidas, larguras das colunas e alturas das linhas, linhas e colunas ocultas, painéis fixos e nomes. Os gráficos, as imagens e outros desenhos, os comentários, as tabelas dinâmicas, as macros, a formatação condicional e a validação de dados ainda não são lidos; uma mensagem na parte superior indica quais destes elementos o ficheiro contém, e uma cópia guardada a partir daqui não os terá. Uma folha de cálculo protegida com palavra-passe não é aberta: abra-a no programa que a criou e guarde uma cópia sem palavra-passe.

Um ficheiro de texto corresponde a uma folha. O JeffCrunch deteta como os valores estão separados (por vírgulas, pontos e vírgulas, tabulações ou barras verticais) e como os carateres estão codificados (UTF-8 ou UTF-16 ou, caso contrário, a página de código do seu idioma), e lê cada valor como o escreveria no seu idioma: um número como 1.234,5 em português, uma percentagem, um montante em dinheiro, uma data com o ano (como 15/03/2024), uma hora, VERDADEIRO ou FALSO (no seu idioma e em inglês, TRUE e FALSE). Um valor que comece por um sinal de igual é uma fórmula. Os algarismos que começam por zero (como 007) e os números com mais de 15 algarismos ficam como texto, para que um código postal ou um número de conta não seja alterado; o mesmo acontece com um dia e um mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12.5, 1-2, 1 de março ou 25 dez), porque o ano teria de ser inventado, e com textos como o nome de gene MARCH1. Uma folha tem 1 048 576 linhas e 16 384 colunas; se o ficheiro tiver mais, o JeffCrunch abre o que couber e informa-o.

Quando abre um ficheiro de texto, uma janela mostra primeiro as primeiras linhas e pergunta como deve ser lido: o que separa os campos (qualquer combinação de vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um carácter que escreva, podendo tratar separadores consecutivos como um só), o delimitador de texto que envolve um campo que contém um separador, a codificação de texto e a linha onde começar. Por cima de cada coluna, escolhe como é lida: **Tal como escrito**, **Texto** (para manter tal como está um número de peça como 007 ou um número de conta longo), uma **Data** pela ordem que indicar, ou **Omitir**. A pré-visualização acompanha todas as alterações e mostra no máximo as primeiras 200 linhas e 100 colunas, mas o ficheiro é aberto na totalidade. **Ficheiro > Abrir > Abrir um ficheiro de texto** faz a pergunta mesmo quando o nome do ficheiro não é habitual.

Para guardar apenas os valores da folha que está a ver, clique em **Ficheiro**, em **Guardar Como** e escolha o tipo de ficheiro **.csv**. O JeffCrunch escreve os valores tal como as células os mostram, em UTF-8, com um ponto e vírgula entre eles nos idiomas que usam a vírgula nos números (como o português) e uma vírgula nos restantes, e com o texto entre aspas quando necessário. As outras folhas, as fórmulas, os formatos e as definições não ficam no ficheiro .csv, e o livro não é alterado ao escrevê-lo.

## Livros grandes

O JeffCrunch lê um livro em segundo plano. As primeiras linhas da folha que estava aberta quando o livro foi guardado aparecem quase de imediato, mesmo quando o livro é muito grande. A barra de estado, no canto inferior esquerdo, indica o que está a fazer: **A abrir**, **A ler** uma folha com o número de células lidas até ao momento, **A calcular** com uma percentagem e, por fim, **Pronto**.

Pode deslocar-se, selecionar e copiar enquanto a leitura decorre. Os separadores das outras folhas ficam disponíveis à medida que são lidas. Um livro danificado é aberto até onde for possível lê-lo, e uma mensagem indica o que ficou de fora.

## Livros recentes

A página **Abrir** do ecrã Ficheiro lista os livros que abriu recentemente. Clique num deles com o botão direito do rato para o **afixar** no topo da lista ou para o remover da lista.

## Guardar

Prima **{key:Save}** ou clique em **Ficheiro** e em **Guardar** para guardar as alterações no livro que abriu. Para manter o original e criar um novo ficheiro, clique em **Ficheiro**, em **Guardar Como** e escolha um nome. Um livro com macros é guardado como ficheiro **.xlsm** e um livro sem macros como ficheiro **.xlsx**.

O JeffCrunch guarda apenas o que alterou. Todo o resto do ficheiro, como as outras folhas, os gráficos, as imagens, as macros e as definições que o JeffCrunch não mostra, é copiado tal como estava, byte a byte. Numa folha que tenha alterado, só são escritas novamente as células que alterou, juntamente com as fórmulas que dependem delas, que recebem os novos valores. Se não alterou nenhuma célula, o ficheiro não é modificado.

Se outro programa tiver alterado o ficheiro depois de o abrir, o JeffCrunch não o substitui sem perguntar, porque isso apagaria as alterações do outro programa. Pergunta se pretende **Guardar uma Cópia**, **Substituir** o ficheiro ou **Cancelar**.

O novo ficheiro é escrito primeiro ao lado do antigo e só ocupa o lugar deste quando estiver completo. Se algo correr mal, é apresentada uma mensagem e o ficheiro existente fica como estava.

Se as fórmulas ainda estiverem a ser calculadas quando guardar, o JeffCrunch aguarda que terminem, para que o ficheiro contenha os respetivos resultados. Se, entretanto, premir **Esc** para interromper o cálculo, a gravação é cancelada e a barra de estado indica-o; guarde novamente quando quiser. O JeffCrunch deixa um elemento de fora do ficheiro: a lista da ordem pela qual um programa calculou as fórmulas pela última vez (a cadeia de cálculo). O programa que abrir o livro a seguir volta a criá-la. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se interrompeu o cálculo, o ficheiro pede a esse programa que recalcule todas as fórmulas ao abrir o livro.

## Fechar com alterações

Se fechar um livro com alterações não guardadas, o JeffCrunch pergunta se pretende **Guardar** as alterações, **Não Guardar** ou **Cancelar** e continuar a trabalhar. Um asterisco (*) no título da janela indica que existem alterações não guardadas.
