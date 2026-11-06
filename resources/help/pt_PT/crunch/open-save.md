# Abrir e guardar pastas de trabalho
<!-- keywords: copiar, formato antigo, opendocument, compatibilidade, novo, em branco, livro1, sem título, abrir, guardar, guardar como, ficheiro, livro, xlsx, xlsm, xls, ods, csv, tsv, ficheiro de texto, delimitado, importar, exportar, recente, afixado, arrastar, soltar, carregamento, grande, macros, não guardado, alterações, fechar, cópia, cadeia de cálculo, modelo, xltx, xltm, não é possível abrir, danificado, cifrado, palavra-passe, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, ots, reparar, abrir e reparar, modelos incorporados, orçamento, fatura, folha de horas, empréstimo, calendário, pauta de notas, largura fixa, colunas fixas, separador decimal, separador de milhares, sinal de menos à direita, html, página web, htm, tabelas de uma página -->

Quando o JeffCrunch é iniciado sem nenhum ficheiro a abrir, a janela contém uma pasta de trabalho nova e vazia, com uma folha chamada Livro1. Cada nova pasta de trabalho recebe o número seguinte (Livro2, Livro3, e assim por diante), e um número não é reutilizado depois de a respetiva janela ser fechada. Para iniciar outra, prima **{key:New}**, ou clique em **Ficheiro**, depois em **Novo**, e escolha **Livro em Branco**; abre-se numa janela própria. Uma nova pasta de trabalho que não tenha alterado cede lugar quando abre um ficheiro na respetiva janela; uma que tenha alterado mantém-se, e o ficheiro abre-se noutra janela. **Guardar** numa nova pasta de trabalho pede um nome, tal como **Guardar Como**, e guarda-a como ficheiro .xlsx. Fechar uma nova pasta de trabalho que não tenha alterado não pede nada.

Para abrir uma pasta de trabalho, prima **{key:Open}**, ou clique em **Ficheiro** e depois em **Abrir**, e escolha **Procurar**. Também pode arrastar uma pasta de trabalho do gestor de ficheiros para a janela. Uma pasta de trabalho já aberta numa janela não é aberta uma segunda vez: essa janela passa para a frente.

O JeffCrunch abre pastas de trabalho **.xlsx** e pastas de trabalho **.xlsm** (as que contêm macros), e os modelos **.xltx** e **.xltm** (um modelo que contém macros). Não executa macros, e mantém-nas intactas: o projeto de macros, a respetiva assinatura e as partes que as referenciam são gravados exatamente como estavam. Um modelo é guardado como modelo, e uma pasta de trabalho com macros como pasta de trabalho com macros, porque o tipo faz parte do ficheiro.

## Outros tipos de ficheiros

O JeffCrunch também abre pastas de trabalho **.xls** mais antigas, folhas de cálculo OpenDocument (**.ods** e o formato plano **.fods**) e ficheiros de texto com valores (**.csv**, **.tsv** e **.txt**). Uma pasta de trabalho aberta a partir de um destes tipos é guardada como novo ficheiro **.xlsx** (ou como cópia **.xls** ou **.ods**: ver abaixo): quando prime **{key:Save}**, o JeffCrunch informa-o e pede um nome, e o ficheiro que abriu nunca é alterado.

Uma folha de cálculo OpenDocument é lida com as respetivas folhas, valores, fórmulas (com os resultados que o ficheiro guardou), formatos de número, tipos de letra, preenchimentos, limites, células unidas, larguras de colunas e alturas de linhas, linhas e colunas ocultas, painéis fixos e nomes. Gráficos, imagens e outros desenhos, comentários, tabelas dinâmicas, macros, formatação condicional e validação de dados ainda não são lidos; uma mensagem no topo indica quais destes o ficheiro tem, e uma cópia guardada a partir daqui não os terá. Uma folha de cálculo protegida com palavra-passe pede-a ao ser aberta: ver [Abrir pastas de trabalho protegidas por palavra-passe](open-password).

Um ficheiro de texto corresponde a uma folha. O JeffCrunch descobre como os valores estão separados (por vírgulas, pontos e vírgulas, tabulações ou barras verticais) e como as letras estão codificadas (UTF-8 ou UTF-16, ou então a página de códigos do seu idioma), e lê cada valor tal como o escreveria no seu idioma: um número como 1.234,5 em alemão, uma percentagem, um montante em dinheiro, uma data com o ano (como 15/03/2024, ou 15.03.2024 em alemão), uma hora, VERDADEIRO ou FALSO (no seu idioma, como WAHR e FALSCH em alemão, e em inglês). Um valor que começa com um sinal de igual é uma fórmula. Algarismos que começam por zero (como 007) e números com mais de 15 algarismos continuam a ser texto, para que um código postal ou um número de conta não seja alterado, e o mesmo acontece com um dia e um mês sem ano, por qualquer ordem e em qualquer idioma (como 3/4, 12.5, 1-2, 1 de março ou 25 dez), porque o ano teria de ser inventado, e com textos como o nome do gene MARCH1. Uma folha tem 1 048 576 linhas e 16 384 colunas; se o ficheiro tiver mais, o JeffCrunch abre o que cabe e informa-o.

Ao abrir um ficheiro de texto, primeiro aparece uma janela com as primeiras linhas, que pergunta como deve ser lido: o que separa os campos (qualquer um de vírgula, ponto e vírgula, tabulação, espaço, barra vertical, ou um carácter que escreva), que aspas delimitam um campo que contém um separador, a codificação do texto (Unicode, as páginas de códigos do Windows, as páginas ISO 8859, Mac Roman e as da Ásia Oriental), e a linha onde começar. Acima de cada coluna escolhe como é lida: **Tal como escrito**, **Texto** (para manter um número de artigo como 007 ou um número de conta longo tal como está), uma **Data** na ordem que indicar, ou **Omitir**. Alguns ficheiros alinham os valores em colunas de igual largura, com espaços entre elas, e sem qualquer separador. Marque **As colunas têm uma largura fixa**, e as primeiras linhas do ficheiro aparecem numa caixa. Clique entre dois carateres para iniciar uma coluna nesse ponto, e clique novamente para a remover; ou escreva as posições em **Colunas começam em**, como 10, 25, 40. Cada parte é cortada nessas posições, e os espaços à volta são retirados. **Separador decimal** e **Separador de milhares** indicam ao JeffCrunch como os números deste ficheiro estão escritos, independentemente do que usa o seu idioma, pelo que 1.234,50 representa mil duzentos e trinta e quatro e meio quando o ficheiro usa a vírgula como separador decimal. Marque **Um sinal de menos pode surgir depois do número** para ficheiros que escrevem -12 como 12-. A pré-visualização acompanha todas as alterações e mostra, no máximo, as primeiras 200 linhas e 100 colunas, mas o ficheiro inteiro é aberto. **Ficheiro > Abrir > Abrir Ficheiro de Texto** pede sempre, mesmo que o nome do ficheiro seja invulgar.

O JeffCrunch também abre **páginas web** (**.html** e **.htm**), e uma página guardada com o nome de uma pasta de trabalho. Cada tabela da página torna-se uma folha, com o nome **Tabela1**, **Tabela2**, e assim por diante. As células são lidas como se as tivesse escrito: um número é um número, e uma data é uma data. Uma célula unida por várias colunas ou linhas deixa células vazias, para que as restantes células permaneçam nas respetivas colunas. O texto fora das tabelas, as imagens e o aspeto da página não são lidos, e uma página sem tabela não abre. Nada que a página refira é transferido da Internet.

Para manter apenas os valores da folha que está a ver, clique em **Ficheiro** e depois em **Guardar Como**, e escolha o tipo de ficheiro **.csv** (ou **.tsv** e **.txt**, separados por tabulações). Antes de escrever o ficheiro, o JeffCrunch indica o que a sua pasta de trabalho tem e que o ficheiro não conterá (as outras folhas, fórmulas, imagens, gráficos, notas e macros, por exemplo), pergunta como o deve escrever, e se deve **Guardar Apenas Valores**, **Guardar como .xlsx** para manter tudo, ou **Cancelar**:

- **Separador.** Uma vírgula, ponto e vírgula, tabulação, espaço, barra vertical, ou um carácter que escreva. Um **.csv** começa com ponto e vírgula onde o seu idioma escreve vírgula nos números, e com vírgula nos restantes casos; um **.tsv** ou **.txt** com tabulação.
- **Codificação do texto.** UTF-8 com marca (para que outros programas leiam corretamente as letras acentuadas), UTF-8 sem marca, UTF-16, ou uma página de códigos como Europa Ocidental (1252), as páginas ISO 8859, Mac Roman, ou as páginas japonesa, chinesa, coreana, hebraica, árabe, báltica, vietnamita e tailandesa. Uma letra para a qual a página de códigos não tem byte é escrita como ponto de interrogação, e a barra de estado indica quantas foram.
- **Finais de linha.** Windows (retorno de carro e avanço de linha), Linux e Mac (avanço de linha), ou Mac antigo (retorno de carro).
- **Células.** **Tal como apresentado nas células** escreve o que vê. **Tal como armazenado** escreve todos os algarismos de um número (0,1 e não 10%), e uma data como texto ISO, como 2026-03-04.

O texto fica entre aspas quando contém o separador, uma aspa, uma quebra de linha, ou um espaço em qualquer extremidade. As outras folhas, as fórmulas, os formatos e as definições não ficam no ficheiro, e a pasta de trabalho não é alterada por escrever um.

Uma pasta de trabalho que veio de um ficheiro de texto é escrita de volta na forma que o ficheiro tinha: o mesmo separador, finais de linha e codificação. Se a guardou como cópia sem alterar uma célula, a cópia é o mesmo ficheiro, byte a byte, e nada é perguntado.

## Guardar uma cópia como .xls ou .ods

Para criar uma cópia que programas mais antigos, ou programas que leem OpenDocument, possam abrir, clique em **Ficheiro** e depois em **Guardar Como**, e escolha o tipo de ficheiro **.xls** ou **.ods**. A cópia é feita a partir da sua pasta de trabalho tal como está agora, com as alterações que ainda não guardou, e contém:

- os valores, com os respetivos textos, números, valores lógicos e erros;
- as fórmulas, com os resultados que têm agora (uma fórmula matricial continua a ser uma fórmula matricial);
- os formatos de número, tipos de letra, preenchimentos, limites, alinhamento, e se uma célula está bloqueada;
- células unidas, larguras de colunas e alturas de linhas, linhas e colunas ocultas, estrutura de linhas e colunas, e painéis fixos;
- as folhas, pela respetiva ordem, com os nomes e indicação das que estão ocultas, e os nomes que definiu (e as áreas de impressão);
- as ligações, e a forma como cada folha é impressa (o papel, as margens, a orientação, a escala, os cabeçalhos e rodapés, e as quebras de página).

Um ficheiro **.xls** contém também tabelas de dados, e mantém as definições de cálculo da pasta de trabalho. Uma cópia não pode conter tudo. Antes de o JeffCrunch escrever o ficheiro, indica o que a sua pasta de trabalho tem e que o ficheiro não conterá: gráficos, imagens e outros desenhos, notas, tabelas dinâmicas, tabelas, formatação condicional, validação de dados, filtros, proteção de folhas, controlos de formulário, macros, e ligações a outras pastas de trabalho (as fórmulas que as usam mantêm os valores que tinham). Uma fórmula que o formato mais antigo não consegue exprimir (uma função que não tem, ou uma célula para lá da coluna IV ou da linha 65 536) é escrita como o valor a que chegou, e a mensagem indica quantas são. O formato mais antigo fica também limitado a 65 536 linhas, 256 colunas, 255 folhas e 32 767 carateres numa célula, e a cerca de 4000 formatos de célula e 56 cores: o que não couber é cortado, e a mensagem indica quanto. Pode então escolher **Guardar Cópia**, **Guardar como .xlsx** para manter tudo, ou **Cancelar**.

A sua pasta de trabalho não é alterada pela cópia, e não fica marcada como guardada. O ficheiro que abriu nunca é escrito: dê à cópia outro nome. Uma cópia de um ficheiro **.xls** ou **.ods** que abriu pode ser guardada no outro tipo.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia** e escolha um nome. O JeffCrunch escreve a pasta de trabalho nesse ficheiro, com as alterações que ainda não guardou, e continua a trabalhar na pasta de trabalho que tinha. O nome e as alterações não guardadas mantêm-se como estavam. Pode escolher qualquer tipo de ficheiro que **Guardar Como** oferece. Uma cópia nunca é escrita por cima do próprio ficheiro da pasta de trabalho.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções**, e na página **Geral** ative **Criar sempre cópia de segurança**. Sempre que guardar, o JeffCrunch copia primeiro o ficheiro antigo para outro chamado "Cópia de segurança de" seguido do nome do ficheiro, na mesma pasta. Se a cópia não puder ser feita, a aplicação pergunta se deve guardar sem ela. A resposta é Não, a menos que diga Sim, e o ficheiro mantém-se como estava.

## Guardar como modelo

Um modelo é uma pasta de trabalho que usa como ponto de partida. Pode conter fórmulas, estilos e folhas. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo (um ficheiro **.xltx**) fica em **Os Meus Modelos**, e a sua pasta de trabalho não é alterada. Em **Guardar Como**, também pode escolher **Modelos (*.xltx)**, ou **Modelos de folha de cálculo OpenDocument (*.ots)**, para guardar um ficheiro de modelo onde preferir.

Para iniciar uma pasta de trabalho a partir do seu modelo, clique em **Ficheiro > Novo**. Os seus modelos aparecem em **Os Meus Modelos**. Clique num deles. Abre-se uma nova pasta de trabalho com as células e folhas do modelo. Ainda não tem ficheiro próprio, pelo que **Guardar** pede um nome, e o modelo não é alterado. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes.

- **Abrir Só de Leitura** abre a pasta de trabalho apenas para leitura. Pode ver, deslocar, pesquisar e copiar, mas não pode alterar uma célula. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffCrunch pede um novo nome, para que o ficheiro que abriu não seja alterado.
- **Abrir como Cópia** abre a pasta de trabalho sem ficheiro próprio. **Guardar** pergunta onde a colocar. O ficheiro que abriu nunca é alterado.
- **Abrir numa Nova Janela** abre a pasta de trabalho numa janela própria, mesmo quando a janela em que está está vazia.
- **Abrir e Reparar** destina-se a um ficheiro .xlsx ou .xls danificado. Ver abaixo.

## Abrir e reparar

Por vezes um ficheiro .xlsx está danificado. Um descarregamento interrompido a meio, ou um disco com erros. Um ficheiro .xlsx é um conjunto de partes mais pequenas, como as folhas, os estilos e o texto partilhado. **Abrir e Reparar** lê todas as partes que consegue. Uma parte interrompida a meio é mantida até ao ponto onde parou, pelo que pode obter apenas as primeiras linhas de uma folha. Uma parte que não pode ser lida é omitida. Se a lista de partes ou as ligações entre elas tiverem desaparecido, o JeffCrunch volta a criá-las.

A pasta de trabalho reparada abre-se sem ficheiro próprio, com um nome como "Orçamento reparado". Uma mensagem indica o que foi mantido até uma interrupção e o que foi omitido. O ficheiro danificado não é alterado. Guarde a pasta de trabalho reparada com um novo nome, e verifique os números antes de confiar neles.

### Um ficheiro .xls

Um ficheiro .xls também pode estar danificado. Se o respetivo invólucro exterior estiver cortado, **Abrir e Reparar** procura no ficheiro o início da pasta de trabalho e lê a partir daí. Uma folha que foi cortada mantém as células anteriores ao corte. Tudo o que vinha depois do corte perde-se, e é informado disso. A pasta de trabalho reparada abre-se como cópia, pelo que deve guardá-la com um novo nome.

## Pastas de trabalho grandes

O JeffCrunch lê uma pasta de trabalho em segundo plano. As primeiras linhas da folha que estava aberta quando a pasta foi guardada aparecem quase de imediato, mesmo que a pasta seja muito grande. A barra de estado, no canto inferior esquerdo, indica o que está a fazer: **A abrir**, **A ler** uma folha com o número de células lidas até ao momento, **A calcular** com uma percentagem, e depois **Pronto**.

Pode deslocar-se, selecionar e copiar enquanto a leitura decorre. Os separadores das outras folhas ficam ativos à medida que são lidos. Uma pasta de trabalho danificada é aberta até onde pode ser lida, e uma mensagem indica o que foi omitido.

## Se um ficheiro não abrir

Quando o JeffCrunch não consegue abrir um ficheiro, diz porquê em linguagem simples e deixa as pastas de trabalho que tem abertas como estão. Distingue um ficheiro vazio de um que não é uma pasta de trabalho (um documento PDF, uma imagem, um documento de texto ou uma apresentação, por exemplo); um pacote zip cortado ou danificado, o que normalmente significa que um descarregamento ou uma cópia foi interrompido, pelo que deve copiá-lo novamente a partir do original; uma pasta de trabalho protegida com palavra-passe, que a pede (ver [Abrir pastas de trabalho protegidas por palavra-passe](open-password)); e um ficheiro composto que não contém nenhuma pasta de trabalho.

## Pastas de trabalho recentes

A página **Abrir** do ecrã Ficheiro lista as pastas de trabalho que abriu recentemente. Clique com o botão direito numa delas para a **afixar** no topo da lista ou para a remover da lista.

## Guardar

Prima **{key:Save}**, ou clique em **Ficheiro** e depois em **Guardar**, para guardar as alterações na pasta de trabalho que abriu. Para manter o original e criar um novo ficheiro, clique em **Ficheiro** e depois em **Guardar Como**, e escolha um nome. Uma pasta de trabalho com macros é guardada como ficheiro **.xlsm**, um modelo como ficheiro **.xltx** ou **.xltm**, e qualquer outra como ficheiro **.xlsx**. Um nome que termine noutro tipo de pasta de trabalho não é aceite, e a mensagem indica qual a terminação a usar; os nomes que terminam em **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** criam cópias, como descrito acima.

O JeffCrunch guarda apenas o que alterou. Tudo o resto do ficheiro, como as outras folhas, os gráficos, as imagens, as macros e as definições que o JeffCrunch não mostra, é copiado tal como estava, byte a byte. Numa folha que alterou, apenas as células alteradas são escritas de novo, juntamente com as fórmulas que dependem delas, que recebem os novos valores. Se não alterou nenhuma célula, o ficheiro fica como está.

Se outro programa alterou o ficheiro depois de o ter aberto, o JeffCrunch não guarda por cima dele sem perguntar, porque isso faria perder as alterações desse outro programa. Pergunta se deve **Guardar uma Cópia**, **Substituir** o ficheiro, ou **Cancelar**.

O novo ficheiro é escrito primeiro ao lado do antigo, e só ocupa o lugar deste quando estiver completo. Se algo correr mal, vê uma mensagem, e o ficheiro que existia mantém-se como estava.

Se as fórmulas ainda estiverem a ser calculadas quando guarda, o JeffCrunch espera que terminem, para que o ficheiro contenha os respetivos resultados. Se premir **Esc** para interromper o cálculo entretanto, o guardar é cancelado, e a barra de estado indica-o; guarde novamente quando quiser. O JeffCrunch deixa uma coisa de fora do ficheiro: a lista da ordem pela qual um programa calculou as fórmulas pela última vez (a cadeia de cálculo). Um programa que abra a pasta de trabalho a seguir volta a criá-la. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se interrompeu o cálculo, o ficheiro pede a esse programa que calcule novamente todas as fórmulas quando abrir a pasta de trabalho.

## Fechar com alterações

Se fechar uma pasta de trabalho que tem alterações não guardadas, o JeffCrunch pergunta se deve **Guardar** essas alterações, **Não Guardar**, ou **Cancelar** e continuar a trabalhar. Um asterisco (*) no título da janela indica que existem alterações não guardadas.

## Modelos incorporados

Em **Ficheiro > Novo**, abaixo da pasta de trabalho em branco, existem pastas de trabalho já preparadas, agrupadas em: **Finanças** (um orçamento mensal, um plano de empréstimo e um registo de cheques), **Negócios** (uma fatura, um relatório de despesas, uma folha de horas, uma lista de inventário, um registo de quilometragem e um relatório de vendas), **Planeamento** (um calendário e um plano de projeto) e **Educação** (uma pauta de notas). Cada uma mostra uma pequena imagem do topo. Clique numa delas para iniciar uma nova pasta de trabalho a partir dela. Escreva nas caixas amarelas. Os restantes números são calculados automaticamente. A pasta de trabalho não tem ficheiro próprio até a guardar.
