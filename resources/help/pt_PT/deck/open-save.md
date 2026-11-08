# Abrir e guardar apresentações
<!-- keywords: abrir, guardar, guardar como, otp, libreoffice, deixado de fora, fechar, novo, ficheiros recentes, afixados, tipos de ficheiro, pptx, potx, ppsx, ppt, odp, modelo, apresentação, renomear, Ctrl+S, cópia de segurança, formatos de ficheiro, ficheiros antigos, documento aberto, mantido, não mostrado, tipos de letra, incorporar tipos de letra, tipos de letra incorporados, tipo de letra em falta, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, reparar, abrir e reparar, danificado -->

O JeffDeck guarda o seu trabalho em ficheiros .pptx e em ficheiros .odp, e abre vários tipos de ficheiros de apresentação.

## Iniciar uma nova apresentação

Prima Ctrl+N. Abre-se uma nova janela com uma apresentação em branco. Ou clique em **Ficheiro > Novo** e, depois, em **Apresentação em Branco**. Se a janela em que está contiver uma apresentação nova que ninguém tenha alterado, a apresentação em branco substitui-a. Caso contrário, abre-se numa janela própria.

## Guardar

Prima Ctrl+S, ou clique em **Ficheiro > Guardar**. Na primeira vez, o JeffDeck pede um nome e um local. Depois disso, Guardar substitui o ficheiro pelo seu trabalho mais recente. Uma mensagem na parte inferior diz "Guardado" e o nome do ficheiro.

Um asterisco (*) depois do nome na barra de título significa que tem alterações que ainda não guardou. Guarde com frequência. Se o JeffDeck parar inesperadamente, consulte [Recuperar trabalho não guardado](recover).

Se uma mensagem disser que alguns objetos "não podem ser guardados neste tipo de ficheiro ainda", o ficheiro é guardado, mas o JeffDeck não tinha forma de escrever esses objetos. Guarde também o ficheiro original. Ao guardar um ficheiro .odp, a mensagem indica o que esse tipo de ficheiro não consegue conter. A secção "Guardar como um ficheiro .odp" abaixo explica o que não pode conter.

## Guardar com um novo nome ou tipo

Clique em **Ficheiro > Guardar Como**, ou prima F12. Escreva um nome, escolha o tipo e clique em **Guardar**. As opções são:

- **Apresentação (*.pptx)** é a escolha normal.
- **Modelo (*.potx)** guarda os diapositivos como padrão para iniciar novas apresentações.
- **Apresentação de diapositivos (*.ppsx)** guarda uma apresentação para ser reproduzida.
- **Apresentação OpenDocument (*.odp)** guarda os diapositivos no tipo de ficheiro que o LibreOffice e outros programas gratuitos usam.
- **Modelo de Apresentação OpenDocument (*.otp)** é a forma de modelo desse tipo.

Se não escrever a extensão, o JeffDeck acrescenta a que corresponde ao tipo escolhido. Depois de guardar, a barra de título mostra o novo nome, e continua a trabalhar no novo ficheiro.

Uma apresentação aberta a partir de um modelo é guardada como nova apresentação: **Guardar** pede um nome, para que o modelo fique como estava. O mesmo acontece com uma apresentação aberta a partir de um ficheiro .ppt, .pps ou .pot mais antigo. Uma apresentação aberta a partir de um ficheiro .odp é guardada novamente como ficheiro .odp, no mesmo ficheiro.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia** e escolha um nome. O JeffDeck grava a apresentação nesse ficheiro, e continua a trabalhar no que tinha aberto. O nome e as alterações por guardar mantêm-se como estavam.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, no separador **Geral**, ative **Criar sempre cópia de segurança**. Sempre que guardar, o JeffDeck copia primeiro o ficheiro antigo para outro com o nome "Cópia de segurança de" seguido do nome do ficheiro, na mesma pasta. Se a cópia não puder ser feita, a aplicação pergunta se deve guardar sem ela. A resposta é Não, a menos que diga Sim, e o ficheiro mantém-se como estava.

## Guardar como modelo

Um modelo é uma apresentação que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo (um ficheiro **.potx**) fica em **Os Meus Modelos**, e a sua apresentação não é alterada. Para iniciar uma nova apresentação a partir dele, clique em **Ficheiro > Novo** e clique no modelo em **Os Meus Modelos**. A nova apresentação ainda não tem ficheiro próprio, por isso o modelo não é alterado. Para retirar um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Abrir

Prima Ctrl+O para escolher logo um ficheiro. Ou clique em **Ficheiro > Abrir** e, depois, clique em **Procurar**, ou num nome em **Afixados** ou **Apresentações Recentes**.

O ficheiro abre-se nesta janela se contiver uma apresentação nova que ninguém tenha alterado. Caso contrário, abre-se numa janela própria.

Clique com o botão direito num nome da lista para **Afixar na lista** (fica no topo), **Desafixar da lista** ou **Remover da lista**. Um nome a cinzento significa que o JeffDeck não consegue encontrar esse ficheiro neste momento.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes.

- **Abrir Só de Leitura** abre a apresentação apenas para leitura. Pode ver, percorrer os diapositivos e copiar, mas não pode alterar nada. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffDeck pede um novo nome, para que o ficheiro que abriu não seja alterado.
- **Abrir como Cópia** abre os diapositivos como uma nova apresentação sem ficheiro próprio. **Guardar** pede onde a colocar.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo que a janela em que está esteja vazia.
- **Abrir e Reparar** destina-se a um ficheiro .pptx danificado, por exemplo um que ficou cortado ao ser copiado. O JeffDeck lê todas as partes que consegue, mantém uma parte que parou a meio até ao ponto onde parou, e deixa de fora uma parte que não consegue ler. Os diapositivos reparados abrem-se como uma cópia chamada "Reparada" seguido do nome, e uma mensagem indica o que foi deixado de fora. O ficheiro danificado não é alterado.

## Tipos de letra no ficheiro

Um tipo de letra é o desenho das letras. Nem todos os computadores têm todos os tipos de letra. Se um computador não tiver um tipo de letra que os seus diapositivos usam, desenha as letras noutro tipo de letra. Nesse caso, os diapositivos podem ter outro aspeto.

O JeffDeck pode colocar os tipos de letra que os seus diapositivos usam dentro do ficheiro. Assim, os diapositivos têm o mesmo aspeto em qualquer computador. Isto chama-se incorporar. O JeffDeck faz isto sempre que guarda. Para desativar, clique em **Ficheiro > Opções**, abra o separador **Guardar** e desmarque **Incorporar tipos de letra no ficheiro**.

- O JeffDeck coloca apenas os tipos de letra que vêm com ele, como Fraunces, Lato e Open Sans. Outros computadores não os têm. Nunca coloca um tipo de letra que todos os computadores têm, como Calibri ou Arial. Deixa de fora um tipo de letra cuja licença indique que não pode ser copiado para um ficheiro.
- O JeffDeck coloca cada estilo que os diapositivos usam: normal, negrito, itálico e negrito itálico. Se os diapositivos usarem apenas o estilo normal, só esse é colocado.
- O JeffDeck coloca todas as letras de cada tipo de letra, e não apenas as que usou. Assim, quem abrir o ficheiro pode continuar a alterar as palavras.
- O ficheiro fica maior. Cada estilo de um tipo de letra acrescenta cerca de 40 a 700 KB (um KB são mil bytes). O ficheiro é comprimido ao ser guardado, por isso cresce cerca de metade desse valor.

Ao abrir um ficheiro que tem tipos de letra incorporados, o JeffDeck usa-os enquanto a apresentação estiver aberta. Os diapositivos têm o aspeto que tinham onde foram criados. O JeffDeck não instala os tipos de letra no seu computador, nem os copia para qualquer lado. Pertencem apenas a essa apresentação: não aparecem nas suas listas de tipos de letra, e nenhuma outra apresentação os usa. Desaparecem quando fechar a apresentação. Se o computador já tiver um tipo de letra, o JeffDeck usa a sua própria cópia.

O JeffDeck lê tipos de letra de ficheiros .pptx criados por outros programas de apresentações, mesmo que o programa os tenha comprimido. Também lê tipos de letra de ficheiros .odp.

Um ficheiro pode conter um tipo de letra que o JeffDeck não tem. Ao guardar, o JeffDeck volta a colocar esse tipo de letra no ficheiro, desde que os diapositivos ainda o usem. Isto acontece quer **Incorporar tipos de letra no ficheiro** esteja ativado quer não. Se um tipo de letra de um ficheiro estiver danificado ou não puder ser lido, a mensagem que aparece ao abrir o ficheiro indica isso.

## Tipos de ficheiros que o JeffDeck lê

- **.pptx, .potx e .ppsx** são um formato em três formas: apresentação, modelo e apresentação de diapositivos. O JeffDeck lê e escreve as três.
- **.ppt, .pps e .pot** são o tipo mais antigo. O JeffDeck lê-os, mas não os consegue escrever, por isso **Guardar** pede um novo nome e guarda um .pptx (um .ppsx no caso de um .pps). Nunca escreve por cima do original.
- **.odp e .otp** são apresentações de documento aberto e os respetivos modelos. O JeffDeck lê e escreve ambos. Um .odp aberto aqui é guardado como .odp. Um .otp é um modelo, por isso **Guardar** pede um novo nome e sugere um .odp.

Se parte de um ficheiro não puder ser lida, o JeffDeck abre o resto e indica o que deixou de fora. Se o ficheiro inteiro não puder ser lido, diz porquê.

## Guardar como um ficheiro .odp

Um ficheiro .odp abre-se no LibreOffice Impress com os seus diapositivos, modelos globais, imagens, notas e comentários, e com o aspeto que o JeffDeck mostra. Contém animações, transições de diapositivos, apresentações personalizadas e as definições da apresentação de diapositivos. O tipo de letra de cada palavra é guardado pelo nome, por isso um computador que não tenha o tipo de letra mostra outro.

O JeffDeck ainda não lê de volta as animações, os comentários, as apresentações personalizadas nem as definições de apresentação a partir de um ficheiro .odp. Estão no ficheiro, e o LibreOffice reproduz-nos, mas quando abre o ficheiro .odp no JeffDeck, este indica que foram deixados de fora. Mantenha o seu ficheiro .pptx se quiser continuar a editá-los aqui.

Um ficheiro .odp não tem lugar para algumas coisas. O JeffDeck indica-as após a gravação, na mensagem na parte inferior da janela:

- **Gráficos, diagramas e equações** são guardados como imagens. Pode vê-los, mas não pode editar os seus números ou partes no ficheiro .odp.
- **Secções não são guardadas.**
- **Chanfros e sombras interiores** são deixados de fora, e o mesmo acontece com brilhos e reflexos em texto. Uma sombra em texto é mantida. Um reflexo numa forma, uma linha com gradação, uma linha de duas ou três espessuras e o tamanho de uma sombra são mantidos de uma forma que só o JeffDeck lê. Outro programa mostra a forma sem o reflexo, a linha na sua primeira cor, um traço único e a sombra à dimensão do objeto.
- **Efeitos de animação** que começam ao clicar num objeto, e efeitos que alteram uma cor, são deixados de fora. Um clique num vídeo, para o pausar ou reproduzir novamente, é mantido.
- **Um vídeo ou um som ligado** a partir de outro ficheiro, e não guardado na apresentação, mantém o seu endereço e a sua imagem no ficheiro .odp. O LibreOffice reproduz-no se conseguir encontrar o ficheiro. Vídeos e sons que estão na apresentação são guardados no ficheiro .odp e reproduzem-se.

Mantenha também o seu ficheiro .pptx, se precisar destas coisas de volta.

## O que é mantido mas não mostrado

Algumas partes de um ficheiro são coisas que o JeffDeck ainda não consegue mostrar nem alterar. Mantém-nas com os respetivos diapositivos e volta a escrevê-las ao guardar, para que não se percam. Incluem animações, que são reproduzidas na apresentação, vídeos e sons, que ainda não são reproduzidos, e um efeito de transição para o qual o JeffDeck não tem forma, que é reproduzido como um desvanecimento. Os gráficos e diagramas são mostrados como foram guardados. Consulte [Bem-vindo ao JeffDeck](start).

## Ver também

- [Recuperar trabalho não guardado](recover)
- [O ecrã Ficheiro](file-screen)
- [Reutilizar diapositivos de outra apresentação](reuse)
- [Imprimir, PDF e imagens dos diapositivos](printing)
