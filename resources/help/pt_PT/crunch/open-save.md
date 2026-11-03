# Abrir e guardar livros
<!-- keywords: cópia, formato antigo, opendocument, compatibilidade, novo, em branco, livro1, sem título, abrir, guardar, guardar como, ficheiro, livro, folha de cálculo, xlsx, xlsm, xls, ods, csv, tsv, ficheiro de texto, delimitado, separado por vírgulas, importar, exportar, recentes, afixados, arrastar, largar, carregar, grande, macros, não guardado, alterações, fechar, cadeia de cálculo, modelo, xltx, xltm, não abre, danificado, encriptado, palavra-passe -->

Quando o JeffCrunch é iniciado sem nenhum ficheiro para abrir, a janela contém um livro novo e vazio com uma folha, chamado Livro1. Cada livro novo recebe o número seguinte (Livro2, Livro3, etc.), e um número não volta a ser utilizado depois de a respetiva janela ser fechada. Para começar outro, prima **{key:New}** ou clique em **Ficheiro**, depois em **Novo**, e escolha **Livro em branco**; este abre numa janela própria. Um livro novo que não alterou dá lugar ao ficheiro que abrir na respetiva janela; um que tenha alterado mantém-se, e o ficheiro abre noutra janela. **Guardar** num livro novo pede um nome, tal como **Guardar Como**, e guarda-o como ficheiro .xlsx. Fechar um livro novo que não alterou não pede confirmação.

Para abrir um livro, prima **{key:Open}** ou clique em **Ficheiro** e depois em **Abrir**, e escolha **Procurar**. Também pode arrastar um livro do gestor de ficheiros para a janela. Um livro que já está aberto numa janela não é aberto uma segunda vez: essa janela passa para a frente.

O JeffCrunch abre livros **.xlsx** e livros **.xlsm** (os que contêm macros), e os modelos **.xltx** e **.xltm** (um modelo que contém macros). Não executa macros, e mantém-nas intactas: o projeto de macros, a respetiva assinatura e as partes que as referem são escritos exatamente como estavam. Um modelo é guardado como modelo, e um livro com macros como livro com macros, porque o tipo faz parte do ficheiro.

## Outros tipos de ficheiros

O JeffCrunch também abre livros **.xls** antigos, folhas de cálculo OpenDocument (**.ods** e o formato simples **.fods**) e ficheiros de texto de valores (**.csv**, **.tsv** e **.txt**). Um livro aberto a partir de um destes é guardado como um novo ficheiro **.xlsx** (ou como uma cópia **.xls** ou **.ods**: veja abaixo): quando prime **{key:Save}**, o JeffCrunch informa-o disso e pede um nome, e o ficheiro que abriu nunca é alterado.

Uma folha de cálculo OpenDocument é lida com as respetivas folhas, valores, fórmulas (com os resultados guardados no ficheiro), formatos de número, tipos de letra, preenchimentos, limites, células unidas, larguras de coluna e alturas de linha, linhas e colunas ocultas, painéis fixos e nomes. Gráficos, imagens e outros desenhos, comentários, tabelas dinâmicas, macros, formatação condicional e validação de dados ainda não são lidos; uma mensagem no topo indica quais destes o ficheiro contém, e uma cópia guardada a partir daqui não os terá. Uma folha de cálculo protegida por palavra-passe pede-a quando a abre: consulte [Abrir livros protegidos por palavra-passe](open-password).

Um ficheiro de texto é uma folha. O JeffCrunch deteta como os valores estão separados (por vírgulas, pontos e vírgulas, tabulações ou barras verticais) e como as letras estão codificadas (UTF-8 ou UTF-16, ou então a página de código do seu idioma), e lê cada valor tal como o escreveria no seu idioma: um número como 1.234,5 em português, uma percentagem, um montante em dinheiro, uma data com o ano (como 15/03/2024), uma hora, VERDADEIRO ou FALSO (no seu idioma e em inglês, TRUE e FALSE). Um valor que começa por um sinal de igual é uma fórmula. Os dígitos que começam por zero (como 007) e os números com mais de 15 dígitos ficam como texto, para que um código postal ou um número de conta não seja alterado, e o mesmo acontece com um dia e um mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12.5, 1-2, 1 de março ou Dec 25), porque o ano teria de ser inventado, e com textos como o nome do gene MARCH1. Uma folha tem 1 048 576 linhas e 16 384 colunas; se o ficheiro tiver mais, o JeffCrunch abre o que cabe e informa-o.

Quando abre um ficheiro de texto, uma janela mostra primeiro as primeiras linhas e pergunta como deve ser lido: o que separa os campos (vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um carácter que escreva, podendo tratar vários seguidos como um só), o que delimita um campo que contém um separador, a codificação de texto (Unicode, as páginas de código do Windows, as páginas ISO 8859, Mac Roman e as da Ásia Oriental) e a linha em que começar. Acima de cada coluna, escolhe como é lida: **Tal como escrito**, **Texto** (para manter uma referência como 007 ou um número de conta longo tal como está), uma **Data** na ordem que indicar, ou **Omitir**. A pré-visualização acompanha todas as alterações e mostra, no máximo, as primeiras 200 linhas e 100 colunas, mas o ficheiro é aberto na totalidade. **Ficheiro > Abrir > Abrir um ficheiro de texto** pergunta mesmo quando o nome do ficheiro é invulgar.

Para guardar apenas os valores da folha que está a ver, clique em **Ficheiro**, depois em **Guardar Como**, e escolha o tipo de ficheiro **.csv** (ou **.tsv** e **.txt**, separados por tabulações). Antes de escrever o ficheiro, o JeffCrunch indica o que o livro tem que o ficheiro não poderá conter (as outras folhas, fórmulas, imagens, gráficos, notas e macros, por exemplo) e pergunta como o deve escrever, e se pretende **Guardar Apenas Valores**, **Guardar como .xlsx** para manter tudo, ou **Cancelar**:

- **Separador.** Vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um carácter que escreva. Um **.csv** começa com ponto e vírgula quando o seu idioma escreve vírgula nos números, e com vírgula nos restantes casos; um **.tsv** ou **.txt**, com tabulação.
- **Codificação de texto.** UTF-8 com marca (para que outros programas interpretem corretamente as letras acentuadas), UTF-8 sem marca, UTF-16, ou uma página de código como Europa Ocidental (1252), as páginas ISO 8859 ou Mac Roman. Uma letra para a qual a página de código não tem byte é escrita como ponto de interrogação, e a barra de estado indica quantas.
- **Fim das linhas.** Windows (retorno de carro e avanço de linha), Linux e Mac (avanço de linha) ou Mac antigo (retorno de carro).
- **Células.** **Tal como apresentado nas células** escreve o que vê. **Tal como armazenado** escreve todos os dígitos de um número (0,1 e não 10%) e uma data como texto ISO, por exemplo 2026-03-04.

O texto fica entre aspas quando contém o separador, uma aspa, uma quebra de linha ou um espaço no início ou no fim. As outras folhas, as fórmulas, os formatos e as definições não ficam no ficheiro, e o livro não é alterado quando escreve um.

Um livro proveniente de um ficheiro de texto é escrito na forma que o ficheiro tinha: o mesmo separador, fim das linhas e codificação. Se o guardou como cópia sem alterar nenhuma célula, a cópia é o mesmo ficheiro, byte a byte, e não é feita nenhuma pergunta.

## Guardar uma cópia como .xls ou .ods

Para criar uma cópia que programas mais antigos, ou programas que leem OpenDocument, consigam abrir, clique em **Ficheiro**, depois em **Guardar Como**, e escolha o tipo de ficheiro **.xls** ou **.ods**. A cópia é criada a partir do livro tal como está agora, com as alterações ainda não guardadas, e contém:

- os valores, com os respetivos textos, números, valores lógicos e erros;
- as fórmulas, com os resultados atuais (uma fórmula de matriz mantém-se fórmula de matriz);
- os formatos de número, tipos de letra, preenchimentos, limites, alinhamento e se uma célula está bloqueada;
- células unidas, larguras de coluna e alturas de linha, linhas e colunas ocultas, destaques de linhas e colunas e painéis fixos;
- as folhas, pela ordem, com os nomes e quais estão ocultas, e os nomes que definiu (e as áreas de impressão);
- as ligações e o modo como cada folha é impressa (papel, margens, orientação, escala, cabeçalhos e rodapés e quebras de página).

Um ficheiro **.xls** contém também tabelas de dados e mantém as definições de cálculo do livro. Uma cópia não pode conter tudo. Antes de escrever o ficheiro, o JeffCrunch indica o que o livro tem que o ficheiro não terá: gráficos, imagens e outros desenhos, notas, tabelas dinâmicas, tabelas, formatação condicional, validação de dados, filtros, proteção da folha, controlos de formulário, macros e ligações a outros livros (as fórmulas que as utilizam mantêm os valores que tinham). Uma fórmula que o formato antigo não consegue exprimir (uma função que não tem, ou uma célula para lá da coluna IV ou da linha 65 536) é escrita como o valor resultante, e a mensagem indica quantas. O formato antigo também está limitado a 65 536 linhas, 256 colunas, 255 folhas e 32 767 carateres por célula, e a cerca de 4000 formatos de célula e 56 cores: o que não cabe é cortado, e a mensagem indica quanto. Pode então escolher **Guardar Cópia**, **Guardar como .xlsx** para manter tudo, ou **Cancelar**.

O livro não é alterado pela cópia nem é marcado como guardado. O ficheiro que abriu nunca é escrito: dê outro nome à cópia. Uma cópia de um ficheiro **.xls** ou **.ods** que abriu pode ser guardada como o outro tipo.

## Livros grandes

O JeffCrunch lê os livros em segundo plano. As primeiras linhas da folha que estava aberta quando o livro foi guardado aparecem quase de imediato, mesmo quando o livro é muito grande. A barra de estado, em baixo à esquerda, indica o que está a fazer: **A abrir**, **A ler** uma folha com o número de células lidas até ao momento, **A calcular** com uma percentagem e, por fim, **Pronto**.

Pode deslocar-se, selecionar e copiar enquanto o livro é lido. Os separadores das outras folhas ficam disponíveis à medida que são lidas. Um livro danificado é aberto até onde for possível lê-lo, e uma mensagem indica o que foi omitido.

## Se um ficheiro não abrir

Quando o JeffCrunch não consegue abrir um ficheiro, explica porquê em linguagem simples e deixa os livros abertos como estão. Distingue um ficheiro vazio de um que não é de todo um livro (um documento PDF, uma imagem, um documento de texto ou uma apresentação, por exemplo); um pacote zip truncado ou danificado, o que normalmente significa que uma transferência ou uma cópia foi interrompida, pelo que deve copiá-lo novamente a partir do original; um livro protegido por palavra-passe, que é pedida (consulte [Abrir livros protegidos por palavra-passe](open-password)); e um ficheiro composto que não contém nenhum livro.

## Livros recentes

A página **Abrir** do ecrã Ficheiro lista os livros que abriu recentemente. Clique com o botão direito do rato num deles para o **afixar** no topo da lista ou para o remover da lista.

## Guardar

Prima **{key:Save}** ou clique em **Ficheiro** e depois em **Guardar** para guardar as alterações no livro que abriu. Para manter o original e criar um ficheiro novo, clique em **Ficheiro** e depois em **Guardar Como**, e escolha um nome. Um livro com macros é guardado como ficheiro **.xlsm**, um modelo como ficheiro **.xltx** ou **.xltm**, e qualquer outro como ficheiro **.xlsx**. Um nome que termine na extensão de outro tipo de livro não é aceite, e a mensagem indica que extensão utilizar; os nomes terminados em **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** criam cópias, conforme descrito acima.

O JeffCrunch guarda apenas o que alterou. Tudo o resto no ficheiro, como as outras folhas, os gráficos, as imagens, as macros e as definições que o JeffCrunch não mostra, é copiado tal como estava, byte a byte. Numa folha que alterou, só as células alteradas são novamente escritas, juntamente com as fórmulas que delas dependem, que recebem os novos valores. Se não alterou nenhuma célula, o ficheiro não é tocado.

Se outro programa alterou o ficheiro depois de o abrir, o JeffCrunch não o substitui sem perguntar, porque isso faria perder as alterações do outro programa. Pergunta se pretende **Guardar uma Cópia**, **Substituir** o ficheiro ou **Cancelar**.

O ficheiro novo é primeiro escrito ao lado do antigo e só o substitui quando está completo. Se algo correr mal, é apresentada uma mensagem e o ficheiro que lá estava fica como estava.

Se as fórmulas ainda estiverem a ser calculadas quando guardar, o JeffCrunch aguarda até terminarem, para que o ficheiro contenha os respetivos resultados. Se premir **Esc** para interromper o cálculo entretanto, a gravação é cancelada e a barra de estado indica-o; guarde novamente quando pretender. O JeffCrunch omite apenas uma coisa do ficheiro: a lista da ordem pela qual um programa calculou as fórmulas pela última vez (a cadeia de cálculo). O programa que abrir o livro a seguir volta a criá-la. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se interrompeu o cálculo, o ficheiro pede a esse programa que volte a calcular todas as fórmulas quando abrir o livro.

## Fechar com alterações

Se fechar um livro com alterações não guardadas, o JeffCrunch pergunta se pretende **Guardar**, **Não Guardar** ou **Cancelar** e continuar a trabalhar. Um asterisco (*) no título da janela indica que existem alterações não guardadas.
