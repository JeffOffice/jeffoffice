# Abrir e guardar apresentações
<!-- keywords: abrir, guardar, guardar como, otp, libreoffice, omitido, fechar, novo, nova apresentação, ficheiros recentes, afixados, tipos de ficheiro, pptx, potx, ppsx, ppt, odp, modelo, apresentação de diapositivos, mudar o nome, Ctrl+S, cópia de segurança, formatos de ficheiro, ficheiros antigos, open document, mantido, não apresentado, tipos de letra, incorporar tipos de letra, tipos de letra incorporados, tipo de letra em falta -->

O JeffDeck guarda o seu trabalho em ficheiros .pptx e em ficheiros .odp, e abre vários tipos de ficheiros de apresentação.

## Iniciar uma nova apresentação

Prima Ctrl+N. É aberta uma nova janela com uma apresentação em branco. Em alternativa, clique em **Ficheiro > Novo** e, em seguida, em **Apresentação em Branco**. Se a janela em que se encontra contiver uma apresentação nova em que ninguém mexeu, a apresentação em branco ocupa o lugar dela. Caso contrário, é aberta numa janela própria.

## Guardar

Prima Ctrl+S ou clique em **Ficheiro > Guardar**. Da primeira vez, o JeffDeck pede um nome e um local. A partir daí, Guardar substitui o ficheiro pelo trabalho mais recente. Uma mensagem na parte inferior indica "Guardado" e o nome do ficheiro.

Um asterisco (*) a seguir ao nome na barra de título indica que existem alterações que não guardou. Guarde com frequência. Se o JeffDeck parar inesperadamente, consulte [Recuperar trabalho não guardado](recover).

Se uma mensagem indicar que alguns objetos "ainda não podem ser guardados neste tipo de ficheiro", o ficheiro é guardado, mas o JeffDeck não conseguiu escrever esses objetos. Mantenha também o ficheiro original. Quando guarda um ficheiro .odp, a mensagem indica o que esse tipo de ficheiro não consegue conter. A secção "Guardar como ficheiro .odp", mais abaixo, descreve o que não consegue conter.

## Guardar com um novo nome ou tipo

Clique em **Ficheiro > Guardar Como** ou prima F12. Escreva um nome, escolha o tipo e clique em **Guardar**. As opções são:

- **Apresentação (*.pptx)** é a opção habitual.
- **Modelo (*.potx)** guarda os diapositivos como base para iniciar novas apresentações.
- **Apresentação de diapositivos (*.ppsx)** guarda uma apresentação para ser reproduzida.
- **Apresentação OpenDocument (*.odp)** guarda os diapositivos no tipo de ficheiro utilizado pelo LibreOffice e por outros programas gratuitos.
- **Modelo de Apresentação OpenDocument (*.otp)** é a versão de modelo desse tipo.

Se não escrever a extensão, o JeffDeck adiciona a que corresponde ao tipo. Depois de guardar, a barra de título mostra o novo nome e continua a trabalhar no novo ficheiro.

Uma apresentação aberta a partir de um modelo é guardada como uma nova apresentação: **Guardar** pede um nome, para que o modelo fique como estava. O mesmo acontece com uma apresentação aberta a partir de um ficheiro .ppt, .pps ou .pot mais antigo. Uma apresentação aberta a partir de um ficheiro .odp é novamente guardada como ficheiro .odp, por cima do mesmo ficheiro.

## Abrir

Prima Ctrl+O para escolher um ficheiro de imediato. Em alternativa, clique em **Ficheiro > Abrir** e, em seguida, clique em **Procurar** ou num nome em **Afixados** ou **Apresentações Recentes**.

O ficheiro é aberto nesta janela se esta contiver uma apresentação nova em que ninguém mexeu. Caso contrário, é aberto numa janela própria.

Clique com o botão direito do rato num nome da lista para **Afixar na lista** (fica no topo), **Desafixar da lista** ou **Remover da lista**. Um nome a cinzento significa que o JeffDeck não consegue encontrar esse ficheiro neste momento.

## Tipos de letra no ficheiro

Um tipo de letra é a forma das letras. Nem todos os computadores têm todos os tipos de letra. Se faltar a um computador um tipo de letra utilizado nos seus diapositivos, as letras são desenhadas noutro tipo de letra. Os diapositivos podem então ficar com um aspeto diferente.

O JeffDeck pode colocar dentro do ficheiro os tipos de letra utilizados nos diapositivos. Assim, os diapositivos ficam iguais em qualquer computador. A isto chama-se incorporar. O JeffDeck fá-lo sempre que guarda. Para desativar esta opção, clique em **Ficheiro > Opções**, abra o separador **Guardar** e desmarque **Incorporar tipos de letra no ficheiro**.

- O JeffDeck incorpora apenas os tipos de letra que o acompanham, como Fraunces, Lato e Open Sans. Os outros computadores não os têm. Nunca incorpora um tipo de letra que todos os computadores têm, como Calibri ou Arial. Omite os tipos de letra cuja licença não permite copiá-los para um ficheiro.
- O JeffDeck incorpora cada estilo utilizado nos diapositivos: normal, negrito, itálico e negrito itálico. Se os diapositivos utilizarem apenas o estilo normal, só esse é incorporado.
- O JeffDeck incorpora todas as letras de cada tipo de letra, e não apenas as que utilizou. Assim, quem abrir o ficheiro pode continuar a alterar o texto.
- O ficheiro fica maior. Cada estilo de um tipo de letra acrescenta cerca de 40 a 700 KB (um KB são mil bytes). O ficheiro é comprimido ao ser guardado, pelo que aumenta cerca de metade disso.

Quando abre um ficheiro com tipos de letra incorporados, o JeffDeck utiliza-os enquanto a apresentação estiver aberta. Os diapositivos ficam com o aspeto que tinham onde foram criados. O JeffDeck não instala os tipos de letra no computador nem os copia para outro lado. Pertencem apenas a essa apresentação: não aparecem nas listas de tipos de letra e nenhuma outra apresentação os utiliza. Desaparecem quando fecha a apresentação. Se o computador já tiver um tipo de letra, o JeffDeck utiliza a sua própria cópia.

O JeffDeck lê tipos de letra de ficheiros .pptx criados por outros programas de apresentações, mesmo quando o programa os comprimiu. Também lê tipos de letra de ficheiros .odp.

Um ficheiro pode conter um tipo de letra que o JeffDeck não tem. Quando guarda, o JeffDeck volta a colocar esse tipo de letra no ficheiro, desde que os diapositivos ainda o utilizem. Fá-lo quer a opção **Incorporar tipos de letra no ficheiro** esteja ativada quer não. Se um tipo de letra de um ficheiro estiver danificado ou não puder ser lido, a mensagem apresentada quando abre o ficheiro indica-o.

## Tipos de ficheiros que o JeffDeck lê

- **.pptx, .potx e .ppsx** são um formato em três versões: uma apresentação, um modelo e uma apresentação de diapositivos. O JeffDeck lê e escreve as três.
- **.ppt, .pps e .pot** são o tipo mais antigo. O JeffDeck lê-os, mas não consegue escrevê-los, pelo que **Guardar** pede um novo nome e guarda um ficheiro .pptx (um .ppsx no caso de um .pps). Nunca substitui o original.
- **.odp e .otp** são apresentações OpenDocument e os respetivos modelos. O JeffDeck lê e escreve ambos. Um ficheiro .odp aberto aqui é guardado como .odp. Um ficheiro .otp é um modelo, pelo que **Guardar** pede um novo nome e sugere um .odp.

Se uma parte de um ficheiro não puder ser lida, o JeffDeck abre o resto e indica o que omitiu. Se não for possível ler o ficheiro inteiro, indica o motivo.

## Guardar como ficheiro .odp

Um ficheiro .odp abre no LibreOffice Impress com os diapositivos, modelos globais, imagens, notas e comentários, e com o aspeto apresentado pelo JeffDeck. Contém também animações, transições de diapositivos, apresentações personalizadas e as definições da apresentação de diapositivos. O tipo de letra de cada palavra é guardado pelo nome, pelo que um computador que não tenha esse tipo de letra mostra outro.

O próprio JeffDeck ainda não lê de volta as animações, os comentários, as apresentações personalizadas nem as definições da apresentação a partir de um ficheiro .odp. Estão no ficheiro e o LibreOffice reprodu-los, mas quando abre o ficheiro .odp no JeffDeck, estes são indicados como omitidos. Mantenha o ficheiro .pptx se quiser continuar a editá-los aqui.

Um ficheiro .odp não tem lugar para algumas coisas. O JeffDeck indica-as depois de guardar, na mensagem na parte inferior da janela:

- **Gráficos, diagramas SmartArt e equações** são guardados como imagens. Pode vê-los, mas não pode editar os respetivos números ou partes no ficheiro .odp.
- **As secções** não são guardadas.
- **Biséis e sombras interiores** são omitidos, tal como o brilho e os reflexos no texto. A sombra no texto é mantida. O reflexo numa forma, uma linha com gradação, uma linha com dois ou três traços e o tamanho de uma sombra são mantidos de uma forma que apenas o JeffDeck lê. Outro programa mostra a forma sem o reflexo, a linha na primeira cor, um único traço e a sombra com o tamanho do objeto.
- **Efeitos de animação** que começam quando clica num objeto e efeitos que alteram uma cor são omitidos. Um clique num vídeo, para o colocar em pausa ou reproduzir novamente, é mantido.
- **Um vídeo ou som ligado** a partir de outro ficheiro, e não guardado na apresentação, mantém o endereço e a imagem no ficheiro .odp. O LibreOffice reprodu-lo se conseguir encontrar o ficheiro. Os vídeos e sons que estão na apresentação são mantidos no ficheiro .odp e são reproduzidos.

Mantenha também o ficheiro .pptx, se precisar de recuperar estes elementos.

## O que é mantido mas não apresentado

Algumas partes de um ficheiro são elementos que o JeffDeck ainda não consegue mostrar nem alterar. O JeffDeck mantém-nos com os respetivos diapositivos e escreve-os de volta quando guarda, para que não se percam. Incluem animações, que são reproduzidas na apresentação, vídeos e sons, que ainda não são reproduzidos, e efeitos de transição para os quais o JeffDeck não tem equivalente, que são reproduzidos como desvanecimento. Os gráficos e diagramas SmartArt são apresentados tal como foram guardados. Consulte [Bem-vindo ao JeffDeck](start).

## Relacionado

- [Recuperar trabalho não guardado](recover)
- [O ecrã Ficheiro](file-screen)
- [Reutilizar diapositivos de outra apresentação](reuse)
- [Imprimir, PDF e imagens de diapositivos](printing)
