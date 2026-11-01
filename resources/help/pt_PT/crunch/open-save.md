# Abrir e guardar livros
<!-- keywords: novo, em branco, livro1, sem título, abrir, guardar, guardar como, ficheiro, livro, folha de cálculo, xlsx, xlsm, xls, ods, csv, tsv, ficheiro de texto, delimitado, separado por vírgulas, importar, exportar, recentes, afixados, arrastar, largar, carregar, grande, macros, não guardado, alterações, fechar, cópia, cadeia de cálculo, modelo, xltx, xltm, não abre, danificado, encriptado, palavra-passe -->

Quando o JeffCrunch é iniciado sem nenhum ficheiro para abrir, a janela contém um livro novo e vazio com uma folha, chamado Livro1. Cada livro novo recebe o número seguinte (Livro2, Livro3, etc.), e um número não volta a ser utilizado depois de a respetiva janela ser fechada. Para começar outro, prima **{key:New}**, ou clique em **Ficheiro**, depois em **Novo**, e escolha **Livro em branco**; este abre numa janela própria. Um livro novo que não tenha alterado dá lugar ao ficheiro que abrir na respetiva janela; um que tenha alterado mantém-se, e o ficheiro abre noutra janela. **Guardar** num livro novo pede um nome, tal como **Guardar Como**, e guarda-o como ficheiro .xlsx. Fechar um livro novo que não alterou não pede nenhuma confirmação.

Para abrir um livro, prima **{key:Open}**, ou clique em **Ficheiro** e depois em **Abrir**, e escolha **Procurar**. Também pode arrastar um livro do gestor de ficheiros para a janela. Um livro que já esteja aberto numa janela não é aberto uma segunda vez: essa janela passa para a frente.

O JeffCrunch abre livros **.xlsx** e livros **.xlsm** (os que contêm macros), bem como os modelos **.xltx** e **.xltm** (um modelo que contém macros). Não executa macros, e mantém-nas intactas: o projeto de macros, a respetiva assinatura e as partes que as referem são escritos exatamente como estavam. Um modelo é guardado como modelo, e um livro com macros como livro com macros, porque o tipo faz parte do ficheiro.

## Outros tipos de ficheiros

O JeffCrunch também abre livros **.xls** mais antigos, folhas de cálculo OpenDocument (**.ods** e o formato simples **.fods**) e ficheiros de texto de valores (**.csv**, **.tsv** e **.txt**). Um livro aberto a partir de um destes é guardado como um novo ficheiro **.xlsx**: quando prime **{key:Save}**, o JeffCrunch informa-o disso e pede um nome, e o ficheiro que abriu nunca é alterado.

Uma folha de cálculo OpenDocument é lida com as respetivas folhas, valores, fórmulas (com os resultados que o ficheiro guardou), formatos de número, tipos de letra, preenchimentos, limites, células unidas, larguras de coluna e alturas de linha, linhas e colunas ocultas, painéis fixos e nomes. Os gráficos, as imagens e outros desenhos, os comentários, as tabelas dinâmicas, as macros, a formatação condicional e a validação de dados ainda não são lidos; uma mensagem no topo indica quais destes o ficheiro tem, e uma cópia guardada a partir daqui não os terá. Uma folha de cálculo protegida com palavra-passe não é aberta: abra-a no programa que a criou e guarde uma cópia sem a palavra-passe.

Um ficheiro de texto corresponde a uma folha. O JeffCrunch deteta como os valores estão separados (por vírgulas, pontos e vírgulas, tabulações ou barras verticais) e como as letras estão codificadas (UTF-8 ou UTF-16, ou então a página de código do seu idioma), e lê cada valor como o escreveria no seu idioma: um número como 1.234,5 em alemão, uma percentagem, um montante em dinheiro, uma data com o ano (como 3/15/2024, ou 15.03.2024 em alemão), uma hora, VERDADEIRO ou FALSO (no seu idioma, como WAHR e FALSCH em alemão, e em inglês). Um valor que começa por um sinal de igual é uma fórmula. Os dígitos que começam por zero (como 007) e os números com mais de 15 dígitos ficam como texto, para que um código postal ou um número de conta não sejam alterados, tal como um dia e um mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12.5, 1-2, March 1 ou Dec 25), porque o ano teria de ser inventado, e textos como o nome de gene MARCH1. Uma folha tem 1 048 576 linhas e 16 384 colunas; se o ficheiro tiver mais, o JeffCrunch abre o que couber e informa-o.

Quando abre um ficheiro de texto, uma janela mostra primeiro as primeiras linhas e pergunta como deve ser lido: o que separa os campos (vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um carácter que escreva, podendo tratar separadores consecutivos como um só), o que delimita um campo que contém um separador, a codificação de texto (Unicode, as páginas de código do Windows, as páginas ISO 8859, Mac Roman e as da Ásia Oriental) e a linha em que começar. Por cima de cada coluna, escolhe como esta é lida: **Tal como escrito**, **Texto** (para manter uma referência como 007 ou um número de conta longo tal como está), uma **Data** pela ordem que indicar, ou **Omitir**. A pré-visualização acompanha todas as alterações e mostra no máximo as primeiras 200 linhas e 100 colunas, mas o ficheiro é aberto na totalidade. **Ficheiro > Abrir > Abrir um ficheiro de texto** faz estas perguntas mesmo quando o nome do ficheiro é pouco habitual.

Para guardar apenas os valores da folha que está a ver, clique em **Ficheiro**, depois em **Guardar Como**, e escolha o tipo de ficheiro **.csv** (ou **.tsv** e **.txt**, separados por tabulações). Antes de escrever o ficheiro, o JeffCrunch indica o que o livro tem que o ficheiro não vai conter (as outras folhas, fórmulas, imagens, gráficos, notas e macros, por exemplo), pergunta como o deve escrever e se pretende **Guardar Apenas Valores**, **Guardar como .xlsx** em alternativa e manter tudo, ou **Cancelar**:

- **Separador.** Vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um carácter que escreva. Um **.csv** começa com ponto e vírgula quando o seu idioma escreve os números com vírgula decimal, e com vírgula nos restantes casos; um **.tsv** ou **.txt**, com tabulação.
- **Codificação de texto.** UTF-8 com marca (para que outros programas leiam corretamente as letras acentuadas), UTF-8 sem marca, UTF-16, ou uma página de código como Europa Ocidental (1252), as páginas ISO 8859 ou Mac Roman. Uma letra para a qual a página de código não tem byte é escrita como ponto de interrogação, e a barra de estado indica quantas há.
- **Fim das linhas.** Windows (retorno de carro e avanço de linha), Linux e Mac (avanço de linha), ou Mac antigo (retorno de carro).
- **Células.** **Tal como apresentado nas células** escreve o que vê. **Tal como armazenado** escreve todos os dígitos de um número (0,1 e não 10%) e uma data como texto ISO, como 2026-03-04.

O texto fica entre aspas quando contém o separador, aspas, uma quebra de linha ou um espaço no início ou no fim. As outras folhas, as fórmulas, os formatos e as definições não ficam no ficheiro, e o livro não é alterado ao escrever um.

Um livro proveniente de um ficheiro de texto é escrito na forma que o ficheiro tinha: o mesmo separador, fim das linhas e codificação. Se o guardou como cópia sem alterar nenhuma célula, a cópia é o mesmo ficheiro, byte a byte, e não é feita nenhuma pergunta.

## Livros grandes

O JeffCrunch lê os livros em segundo plano. As primeiras linhas da folha que estava aberta quando o livro foi guardado aparecem quase de imediato, mesmo quando o livro é muito grande. A barra de estado, no canto inferior esquerdo, indica o que está a fazer: **A abrir**, **A ler** uma folha com o número de células lidas até ao momento, **A calcular** com uma percentagem e, por fim, **Pronto**.

Pode deslocar-se, selecionar e copiar enquanto a leitura decorre. Os separadores das outras folhas ficam ativos à medida que estas são lidas. Um livro danificado é aberto até onde for possível lê-lo, e uma mensagem indica o que ficou de fora.

## Se um ficheiro não abrir

Quando o JeffCrunch não consegue abrir um ficheiro, explica porquê em palavras simples e deixa os livros que tem abertos como estão. Distingue um ficheiro vazio de um que não é de todo um livro (um documento PDF, uma imagem, um documento de texto ou uma apresentação, por exemplo); um pacote zip truncado ou danificado, o que normalmente significa que uma transferência ou cópia foi interrompida, pelo que deve copiá-lo novamente a partir do original; um livro encriptado com palavra-passe, que o JeffCrunch ainda não consegue abrir; e um ficheiro composto que não contém nenhum livro.

## Livros recentes

A página **Abrir** do ecrã Ficheiro lista os livros que abriu recentemente. Clique com o botão direito do rato num deles para o **afixar** no topo da lista ou para o remover da lista.

## Guardar

Prima **{key:Save}**, ou clique em **Ficheiro** e depois em **Guardar**, para guardar as alterações no livro que abriu. Para manter o original e criar um novo ficheiro, clique em **Ficheiro** e depois em **Guardar Como**, e escolha um nome. Um livro com macros é guardado como ficheiro **.xlsm**, um modelo como ficheiro **.xltx** ou **.xltm**, e qualquer outro como ficheiro **.xlsx**. Um nome que termine noutra extensão não é aceite, e a mensagem indica qual a extensão a utilizar.

O JeffCrunch guarda apenas o que alterou. Tudo o resto no ficheiro, como as outras folhas, os gráficos, as imagens, as macros e as definições que o JeffCrunch não mostra, é copiado tal como estava, byte a byte. Numa folha que alterou, só são escritas novamente as células que alterou, juntamente com as fórmulas que dependem delas, que recebem os novos valores. Se não alterou nenhuma célula, o ficheiro fica intacto.

Se outro programa tiver alterado o ficheiro depois de o abrir, o JeffCrunch não o substitui sem perguntar, porque isso faria perder as alterações do outro programa. Pergunta se pretende **Guardar uma Cópia**, **Substituir** o ficheiro ou **Cancelar**.

O novo ficheiro é primeiro escrito ao lado do antigo, e só ocupa o lugar deste quando estiver completo. Se algo correr mal, é apresentada uma mensagem, e o ficheiro que lá estava fica como estava.

Se as fórmulas ainda estiverem a ser calculadas quando guardar, o JeffCrunch aguarda que terminem, para que o ficheiro contenha os respetivos resultados. Se, entretanto, premir **Esc** para interromper o cálculo, a gravação é cancelada e a barra de estado indica-o; guarde novamente quando pretender. O JeffCrunch deixa uma coisa de fora do ficheiro: a lista da ordem pela qual um programa calculou as fórmulas da última vez (a cadeia de cálculo). O programa que abrir o livro a seguir volta a criá-la. Se o JeffCrunch não tiver conseguido calcular alguma fórmula, ou se tiver interrompido o cálculo, o ficheiro pede a esse programa que recalcule todas as fórmulas quando abrir o livro.

## Fechar com alterações

Se fechar um livro com alterações não guardadas, o JeffCrunch pergunta se pretende **Guardar** as alterações, **Não Guardar** ou **Cancelar** e continuar a trabalhar. Um asterisco (*) no título da janela indica que existem alterações não guardadas.
