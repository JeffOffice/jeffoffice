# Abrir e guardar livros

<!-- keywords: recuperar trabalho não guardado, recuperação automática, falha, cópia, formato antigo, opendocument, compatibilidade, novo, em branco, livro1, sem título, abrir, guardar, guardar como, ficheiro, livro, xlsx, xlsm, xls, ods, csv, tsv, ficheiro de texto, delimitado, importar, exportar, recentes, afixados, arrastar, largar, carregar, grande, macros, não guardado, alterações, fechar, cópia, cadeia de cálculo, modelo, xltx, xltm, não é possível abrir, danificado, encriptado, palavra-passe, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, ots, reparar, abrir e reparar, modelos incorporados, orçamento, fatura, folha de horas, empréstimo, calendário, pauta de notas, largura fixa, colunas fixas, separador decimal, separador de milhares, sinal de menos à direita, html, página web, htm, tabelas de uma página -->

Quando o JeffCrunch é iniciado sem nenhum ficheiro para abrir, a janela apresenta um livro novo e vazio, com uma folha chamada Livro1. Cada novo livro recebe o número seguinte (Livro2, Livro3, e assim sucessivamente), e um número não volta a ser usado depois de a respetiva janela ser fechada. Para iniciar outro, prima **{key:New}**, ou clique em **Ficheiro** e depois em **Novo**, e escolha **Livro em branco**; abre-se numa janela própria. Um livro novo que não tenha alterado cede o lugar quando abre um ficheiro na sua janela; um que tenha alterado mantém-se, e o ficheiro abre-se noutra janela. **Guardar** num livro novo pede um nome, tal como **Guardar Como**, e guarda-o como ficheiro .xlsx. Fechar um livro novo que não tenha alterado não pede nada.

Para abrir um livro, prima **{key:Open}**, ou clique em **Ficheiro** e depois em **Abrir**, e escolha **Procurar**. Também pode arrastar um livro do gestor de ficheiros para a janela. Um livro que já esteja aberto numa janela não é aberto uma segunda vez: essa janela passa para a frente.

O JeffCrunch abre livros **.xlsx** e livros **.xlsm** (os que contêm macros), e os modelos **.xltx** e **.xltm** (um modelo que contém macros). Não executa macros, e mantém-nas intactas: o projeto de macros, a respetiva assinatura e as partes que as referenciam são gravados exatamente como estavam. Um modelo é guardado como modelo, e um livro com macros como livro com macros, porque o tipo faz parte do ficheiro.

## Recuperar trabalho não guardado

Se o JeffCrunch fechar inesperadamente, por exemplo numa falha de energia, o arranque seguinte mostra uma lista das cópias que guardou do seu trabalho não guardado. Escolha as que pretende abrir e clique em **Abrir**; cada uma abre-se como um livro novo que ainda não tem ficheiro, por isso clique em **Guardar** e escolha um nome. **Eliminar** descarta as cópias marcadas, e **Agora Não** deixa-as para mais tarde. Pode pedir a lista a qualquer momento com **Ficheiro > Abrir > Recuperar Trabalho Não Guardado**. Consulte [Alterar as definições do JeffCrunch](options) para saber com que frequência são feitas as cópias.

## Outros tipos de ficheiros

O JeffCrunch também abre livros **.xls** mais antigos, folhas de cálculo OpenDocument (**.ods** e a forma simples **.fods**) e ficheiros de texto com valores (**.csv**, **.tsv** e **.txt**). Um livro aberto a partir de um destes ficheiros é guardado como um novo ficheiro **.xlsx** (ou como uma cópia **.xls** ou **.ods**: veja abaixo). Quando premir **{key:Save}**, o JeffCrunch avisa-o disso e pede um nome, e o ficheiro que abriu nunca é alterado.

Uma folha de cálculo OpenDocument é lida com as respetivas folhas, valores, fórmulas (com os resultados que o ficheiro guardou), formatos de número, tipos de letra, preenchimentos, limites, células unidas, larguras de coluna e alturas de linha, linhas e colunas ocultas, painéis fixos e nomes. Gráficos, imagens e outros desenhos, comentários, tabelas dinâmicas, macros, formatação condicional e validação de dados ainda não são lidos; uma mensagem no topo indica quais destes o ficheiro tem, e uma cópia guardada a partir daqui não os terá. Uma folha de cálculo protegida com palavra-passe pede-a ao ser aberta: consulte [Abrir livros protegidos por palavra-passe](open-password).

Um ficheiro de texto corresponde a uma folha. O JeffCrunch descobre como os valores estão separados (por vírgulas, pontos e vírgula, tabulações ou barras verticais) e como as letras estão codificadas (UTF-8 ou UTF-16, ou então a página de códigos do seu idioma), e lê cada valor como o escreveria no seu idioma: um número como 1.234,5 em alemão, uma percentagem, um montante em dinheiro, uma data com o ano (como 15/03/2024, ou 15.03.2024 em alemão), uma hora, VERDADEIRO ou FALSO (no seu idioma, e como WAHR e FALSCH em alemão, ou TRUE e FALSE em inglês). Um valor que começa com um sinal de igual é uma fórmula. Algarismos que começam por zero (como 007) e números com mais de 15 algarismos continuam a ser texto, para que um código postal ou um número de conta não seja alterado; o mesmo acontece com um dia e um mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12.5, 1-2, 1 de março ou 25 dez), porque o ano teria de ser inventado, e com textos como o nome do gene MARCH1. Uma folha tem 1 048 576 linhas e 16 384 colunas; se o ficheiro tiver mais, o JeffCrunch abre o que cabe e informa-o.

Ao abrir um ficheiro de texto, uma janela mostra primeiro as primeiras linhas e pergunta como deve ser lido: o que separa os campos (uma vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um carácter que escreva, podendo considerar como um só vários separadores seguidos), o que delimita um campo que contém um separador, a codificação do texto (Unicode, as páginas de códigos do Windows, as páginas ISO 8859, Mac Roman e as do Extremo Oriente) e a linha a partir da qual começar. Acima de cada coluna escolhe como é lida: **Como escrito**, **Texto** (para manter um número de artigo como 007 ou um número de conta longo tal como está), uma **Data** na ordem que indicar, ou **Omitir**. Alguns ficheiros alinham os valores em colunas de larguras iguais, com espaços entre eles e sem qualquer separador. Marque **As colunas têm uma largura fixa**, e as primeiras linhas do ficheiro são mostradas numa caixa. Clique entre dois carateres para iniciar uma coluna nesse ponto, e clique de novo para a retirar; ou escreva as posições em **Colunas começam em**, como 10, 25, 40. Cada parte é cortada nesses pontos, e retiram-se os espaços à volta dela. **Separador decimal** e **Separador de milhares** indicam ao JeffCrunch como os números deste ficheiro estão escritos, seja qual for o idioma que usa, para que 1.234,50 seja mil duzentos e trinta e quatro vírgula cinquenta quando o ficheiro usa a vírgula como separador decimal. Marque **Um sinal de menos pode surgir depois do número** para ficheiros que escrevem -12 como 12-. A pré-visualização acompanha todas as alterações e mostra, no máximo, as primeiras 200 linhas e 100 colunas, mas o ficheiro inteiro é aberto. **Ficheiro > Abrir > Abrir Ficheiro de Texto** pergunta mesmo quando o nome do ficheiro é pouco habitual.

O JeffCrunch também abre **páginas web** (**.html** e **.htm**), e uma página guardada com o nome de um livro. Cada tabela da página torna-se uma folha, com o nome **Tabela1**, **Tabela2**, e assim sucessivamente. As células são lidas como se as tivesse escrito: um número é um número, e uma data é uma data. Uma célula unida por várias colunas ou linhas deixa células vazias, para que as restantes células permaneçam nas respetivas colunas. O texto fora das tabelas, as imagens e o aspeto da página não são lidos, e uma página sem tabela não se abre. Nada que a página referencie é transferido da Internet.

Para guardar apenas os valores da folha que está a ver, clique em **Ficheiro** e depois em **Guardar Como**, e escolha o tipo de ficheiro **.csv** (ou **.tsv** e **.txt**, separados por tabulações). Antes de escrever o ficheiro, o JeffCrunch indica o que o livro tem e o ficheiro não vai conter (as outras folhas, fórmulas, imagens, gráficos, notas e macros, por exemplo), pergunta como o deve escrever, e se deve **Guardar Apenas Valores**, **Guardar como .xlsx** para manter tudo, ou **Cancelar**:

- **Separador.** Uma vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um carácter que escreva. Um **.csv** começa com ponto e vírgula onde o seu idioma escreve uma vírgula nos números, e com vírgula nos restantes casos; um **.tsv** ou **.txt** com tabulação.
- **Codificação do texto.** UTF-8 com marca (para que outros programas leiam corretamente as letras acentuadas), UTF-8 sem marca, UTF-16, ou uma página de códigos como Europa Ocidental (1252), as páginas ISO 8859, Mac Roman, ou as páginas japonesa, chinesa, coreana, hebraica, árabe, báltica, vietnamita e tailandesa. Uma letra para a qual a página de códigos não tem byte é escrita como ponto de interrogação, e a barra de estado indica quantas foram.
- **Finais de linha.** Windows (retorno de carro e avanço de linha), Linux e Mac (avanço de linha), ou Mac antigo (retorno de carro).
- **Células.** **Tal como apresentado nas células** escreve o que vê. **Tal como armazenado** escreve todos os algarismos de um número (0,1 e não 10%), e uma data como texto ISO, como 2026-03-04.

O texto fica entre aspas quando contém o separador, uma aspa, uma quebra de linha ou um espaço em qualquer das extremidades. As outras folhas, as fórmulas, os formatos e as definições não ficam no ficheiro, e o seu livro não é alterado ao escrever um.

Um livro que veio de um ficheiro de texto é gravado de novo na forma que o ficheiro tinha: o mesmo separador, finais de linha e codificação. Se o guardou como cópia sem alterar nenhuma célula, a cópia é o mesmo ficheiro, byte a byte, e nada é perguntado.

## Guardar uma cópia como .xls ou .ods

Para criar uma cópia que programas mais antigos, ou programas que leem OpenDocument, consigam abrir, clique em **Ficheiro** e depois em **Guardar Como**, e escolha o tipo de ficheiro **.xls** ou **.ods**. A cópia é feita a partir do livro tal como está neste momento, com as alterações que ainda não guardou, e contém:

- os valores, com os respetivos textos, números, valores lógicos e erros;
- as fórmulas, com os resultados que têm agora (uma fórmula matricial continua a ser uma fórmula matricial);
- os formatos de número, tipos de letra, preenchimentos, limites, alinhamento, e se uma célula está bloqueada;
- as células unidas, larguras de coluna e alturas de linha, linhas e colunas ocultas, estrutura de linhas e colunas, e painéis fixos;
- as folhas, pela sua ordem, com os respetivos nomes e indicação das que estão ocultas, e os nomes que definiu (e as áreas de impressão);
- as ligações, e a forma como cada folha é impressa (o papel, as margens, a orientação, a escala, os cabeçalhos e rodapés, e as quebras de página).

Um ficheiro **.xls** também contém tabelas de dados, e mantém as definições de cálculo do livro. Uma cópia não consegue conter tudo. Antes de o JeffCrunch escrever o ficheiro, indica o que o livro tem e o ficheiro não vai conter: gráficos, imagens e outros desenhos, notas, tabelas dinâmicas, tabelas, formatação condicional, validação de dados, filtros, proteção de folhas, controlos de formulário, macros e ligações a outros livros (as fórmulas que os usam mantêm os valores que tinham). Uma fórmula que a forma antiga não consegue representar (uma função que não existe nela, ou uma célula para lá da coluna IV ou da linha 65 536) é escrita como o valor a que chegou, e a mensagem indica quantas são. A forma antiga também fica limitada a 65 536 linhas, 256 colunas, 255 folhas e 32 767 carateres numa célula, e a cerca de 4000 formatos de célula e 56 cores: o que não couber é cortado, e a mensagem indica quanto. Pode então optar por **Guardar Cópia**, por **Guardar como .xlsx** para manter tudo, ou por **Cancelar**.

O seu livro não é alterado pela cópia, e não fica marcado como guardado. O ficheiro que abriu nunca é escrito: dê um nome diferente à cópia. Uma cópia de um ficheiro **.xls** ou **.ods** que abriu pode ser guardada no outro tipo.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia** e escolha um nome. O JeffCrunch escreve o livro nesse ficheiro, com as alterações que ainda não guardou, e continua a trabalhar no livro que tinha. O nome e as alterações não guardadas mantêm-se como estavam. Pode escolher qualquer tipo de ficheiro que **Guardar Como** oferece. Uma cópia nunca é escrita por cima do próprio ficheiro do livro.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, na página **Geral**, ative **Criar sempre cópia de segurança**. Sempre que guardar, o JeffCrunch copia primeiro o ficheiro antigo para outro chamado "Cópia de segurança de" seguido do nome do ficheiro, na mesma pasta. Se a cópia não puder ser feita, a aplicação pergunta se deve guardar sem ela. A resposta é Não, a menos que diga Sim, e o ficheiro mantém-se como estava.

## Guardar como modelo

Um modelo é um livro que usa como ponto de partida. Pode conter fórmulas, estilos e folhas. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo (um ficheiro **.xltx**) é guardado em **Os Meus Modelos**, e o seu livro não é alterado. Em **Guardar Como**, também pode escolher **Modelos (*.xltx)**, ou **Modelos de folha de cálculo OpenDocument (*.ots)**, para guardar um ficheiro de modelo na pasta que preferir.

Para iniciar um livro a partir do seu modelo, clique em **Ficheiro > Novo**. Os seus modelos aparecem em **Os Meus Modelos**. Clique num deles. Abre-se um livro novo com as células e as folhas do modelo. Ainda não tem ficheiro próprio, por isso **Guardar** pede um nome, e o modelo não é alterado. Para retirar um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes.

- **Abrir Só de Leitura** abre o livro apenas para leitura. Pode ver, deslocar, procurar e copiar, mas não pode alterar nenhuma célula. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffCrunch pede um novo nome, para que o ficheiro que abriu não seja alterado.
- **Abrir como Cópia** abre o livro sem ficheiro próprio. **Guardar** pede onde o colocar. O ficheiro que abriu nunca é alterado.
- **Abrir numa Nova Janela** abre o livro numa janela própria, mesmo quando a janela em que está está vazia.
- **Abrir e Reparar** destina-se a um ficheiro .xlsx ou .xls danificado. Veja abaixo.

## Abrir e Reparar

Por vezes um ficheiro .xlsx está danificado. Um download interrompido a meio, ou um disco com erros. Um ficheiro .xlsx é um conjunto de partes mais pequenas, como as folhas, os estilos e o texto partilhado. **Abrir e Reparar** lê todas as partes que consegue. Uma parte interrompida a meio é mantida até ao ponto em que parou, pelo que pode obter apenas as primeiras linhas de uma folha. Uma parte que não consegue ser lida é omitida. Se a lista de partes ou as ligações entre elas tiverem desaparecido, o JeffCrunch volta a criá-las.

O livro reparado abre-se sem ficheiro próprio, com um nome como "Orçamento reparado". Uma mensagem indica o que foi mantido até uma interrupção e o que foi omitido. O ficheiro danificado não é alterado. Guarde o livro reparado com um novo nome e verifique os números antes de confiar neles.

### Um ficheiro .xls

Um ficheiro .xls também pode estar danificado. Se o seu invólucro exterior estiver truncado, **Abrir e Reparar** procura no ficheiro o início do livro e lê a partir daí. Uma folha que foi truncada mantém as células anteriores ao corte. Tudo o que vinha depois do corte perde-se, e é informado disso. O livro reparado abre-se como cópia, por isso guarde-o com um novo nome.

## Livros grandes

O JeffCrunch lê um livro em segundo plano. As primeiras linhas da folha que estava aberta quando o livro foi guardado aparecem quase de imediato, mesmo quando o livro é muito grande. A barra de estado, no canto inferior esquerdo, indica o que está a fazer: **A abrir**, **A ler** uma folha com o número de células lidas até ao momento, **A calcular** com uma percentagem, e depois **Pronto**.

Pode deslocar-se, selecionar e copiar enquanto a leitura decorre. Os separadores das outras folhas ficam ativos à medida que são lidos. Um livro danificado é aberto até onde pode ser lido, e uma mensagem indica o que ficou por ler.

## Se um ficheiro não abrir

Quando o JeffCrunch não consegue abrir um ficheiro, indica o motivo em termos simples e deixa os livros que tem abertos como estão. Distingue um ficheiro vazio de um que não é de todo um livro (um documento PDF, uma imagem, um documento de texto ou uma apresentação, por exemplo); um pacote zip truncado ou danificado, o que normalmente significa que um download ou uma cópia foram interrompidos, pelo que deve copiá-lo outra vez a partir do original; um livro protegido por palavra-passe, que o JeffCrunch pede (consulte [Abrir livros protegidos por palavra-passe](open-password)); e um ficheiro composto que não contém nenhum livro.

## Livros recentes

A página **Abrir** do ecrã Ficheiro lista os livros que abriu recentemente. Clique com o botão direito num deles para o **afixar** no topo da lista ou para o retirar da lista.

## Guardar

Prima **{key:Save}**, ou clique em **Ficheiro** e depois em **Guardar**, para guardar as alterações no livro que abriu. Para manter o original e criar um novo ficheiro, clique em **Ficheiro** e depois em **Guardar Como**, e escolha um nome. Um livro com macros é guardado como ficheiro **.xlsm**, um modelo como ficheiro **.xltx** ou **.xltm**, e qualquer outro como ficheiro **.xlsx**. Um nome que termine noutro tipo de livro não é aceite, e a mensagem indica qual o final a usar; os nomes que terminam em **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** criam cópias, como descrito acima.

O JeffCrunch guarda apenas o que alterou. Tudo o resto do ficheiro, como as outras folhas, os gráficos, as imagens, as macros e as definições que o JeffCrunch não mostra, é copiado tal como estava, byte a byte. Numa folha que alterou, só são escritas de novo as células que alterou, juntamente com as fórmulas que dependem delas, que recebem os novos valores. Se não alterou nenhuma célula, o ficheiro fica como está.

Se outro programa alterou o ficheiro depois de o abrir, o JeffCrunch não o substitui sem perguntar, porque isso faria perder as alterações desse programa. Pergunta se deve **Guardar uma Cópia**, **Substituir** o ficheiro, ou **Cancelar**.

O novo ficheiro é primeiro escrito ao lado do antigo, e só ocupa o lugar deste quando estiver completo. Se algo correr mal, vê uma mensagem, e o ficheiro que estava lá mantém-se como estava.

Se as fórmulas ainda estiverem a ser calculadas quando guardar, o JeffCrunch espera que terminem, para que o ficheiro contenha os respetivos resultados. Se entretanto premir **Esc** para interromper o cálculo, o guardar é cancelado, e a barra de estado indica-o; guarde de novo quando quiser. O JeffCrunch deixa de fora do ficheiro uma coisa: a lista da ordem pela qual um programa calculou as fórmulas da última vez (a cadeia de cálculo). Um programa que abra o livro a seguir volta a criá-la. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se interrompeu o cálculo, o ficheiro pede a esse programa que calcule todas as fórmulas de novo ao ser aberto.

## Fechar com alterações

Se fechar um livro que tem alterações não guardadas, o JeffCrunch pergunta se deve **Guardar** essas alterações, **Não Guardar**, ou **Cancelar** e continuar a trabalhar. Um asterisco (*) no título da janela indica que existem alterações não guardadas.

## Modelos incorporados

Em **Ficheiro > Novo**, abaixo do livro em branco, estão livros prontos a usar, agrupados em categorias: **Finanças** (um orçamento mensal, um plano de empréstimo e um registo de cheques), **Negócios** (uma fatura, um relatório de despesas, uma folha de horas, uma lista de inventário, um registo de quilometragem e um relatório de vendas), **Planeamento** (um calendário e um plano de projeto) e **Formação** (uma pauta de notas). Cada um mostra uma pequena imagem do respetivo topo. Clique num para iniciar um novo livro a partir dele. Escreva nas caixas amarelas. Os restantes números são calculados automaticamente. O livro não tem ficheiro próprio até o guardar.
