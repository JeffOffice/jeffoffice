# Abrir e guardar apresentações
<!-- keywords: abrir, guardar, guardar como, otp, libreoffice, deixado de fora, fechar, novo, ficheiros recentes, afixados, tipos de ficheiro, pptx, potx, ppsx, ppt, odp, modelo, apresentar, mudar o nome, Ctrl+S, cópia de segurança, formatos de ficheiro, ficheiros antigos, documento aberto, mantido, não mostrado, tipos de letra, incorporar tipos de letra, tipos de letra incorporados, tipo de letra em falta, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, reparar, abrir e reparar, danificado -->

O JeffDeck guarda o seu trabalho em ficheiros .pptx e em ficheiros .odp, e abre vários tipos de ficheiros de apresentação.

## Iniciar uma nova apresentação

Prima Ctrl+N. Abre-se uma nova janela com uma apresentação em branco. Ou clique em **Ficheiro > Novo** e depois em **Apresentação em Branco**. Se a janela em que está contiver uma apresentação nova que ninguém tocou, a apresentação em branco substitui-a. Caso contrário, abre-se numa janela própria.

## Guardar

Prima Ctrl+S, ou clique em **Ficheiro > Guardar**. Na primeira vez, o JeffDeck pede um nome e um local. Depois disso, Guardar substitui o ficheiro pelo seu trabalho mais recente. Uma mensagem na parte inferior diz "Guardado" e o nome do ficheiro.

Um asterisco (*) depois do nome na barra de título significa que tem alterações não guardadas. Guarde com frequência. Se o JeffDeck parar inesperadamente, consulte [Recuperar trabalho não guardado](recover).

Se uma mensagem disser que alguns objetos "não podem ser guardados neste tipo de ficheiro ainda", o ficheiro é guardado, mas o JeffDeck não tinha forma de gravar esses objetos. Guarde também o ficheiro original. Ao guardar um ficheiro .odp, a mensagem indica o que esse tipo de ficheiro não consegue conter. A secção "Guardar como ficheiro .odp" abaixo explica o que não pode conter.

## Guardar com um novo nome ou tipo

Clique em **Ficheiro > Guardar como**, ou prima F12. Escreva um nome, escolha o tipo e clique em **Guardar**. As opções são:

- **Apresentação (*.pptx)** é a escolha normal.
- **Modelo (*.potx)** guarda os diapositivos como padrão para iniciar novas apresentações.
- **Apresentação de diapositivos (*.ppsx)** guarda uma apresentação para ser reproduzida.
- **Apresentação OpenDocument (*.odp)** guarda os diapositivos no tipo de ficheiro que o LibreOffice e outros programas livres utilizam.
- **Modelo de Apresentação OpenDocument (*.otp)** é a forma de modelo desse tipo.

Se omitir a extensão, o JeffDeck acrescenta a que corresponde ao tipo escolhido. Depois de guardar, a barra de título mostra o novo nome, e continua a trabalhar no novo ficheiro.

Uma apresentação que abriu a partir de um modelo é guardada como uma nova apresentação: **Guardar** pede um nome, para que o modelo fique como estava. O mesmo acontece com uma apresentação aberta a partir de um ficheiro .ppt, .pps ou .pot mais antigo. Uma apresentação aberta a partir de um ficheiro .odp é guardada novamente como .odp, no mesmo ficheiro.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia** e escolha um nome. O JeffDeck grava a apresentação nesse ficheiro, e continua a trabalhar no que tinha aberto. O nome e as alterações não guardadas mantêm-se como estavam.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, no separador **Geral**, ative **Criar sempre cópia de segurança**. Cada vez que guardar, o JeffDeck copia primeiro o ficheiro antigo para outro com o nome "Cópia de segurança de" seguido do nome do ficheiro, na mesma pasta. Se não for possível criar a cópia, a aplicação pergunta se deve guardar sem ela. A resposta é Não, a não ser que diga Sim, e o ficheiro mantém-se como estava.

## Guardar como modelo

Um modelo é uma apresentação que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo (um ficheiro **.potx**) fica em **Os Meus Modelos**, e a sua apresentação não é alterada. Para iniciar uma nova apresentação a partir dele, clique em **Ficheiro > Novo** e clique no modelo em **Os Meus Modelos**. A nova apresentação ainda não tem ficheiro próprio, por isso o modelo não é alterado. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Abrir

Prima Ctrl+O para escolher logo um ficheiro. Ou clique em **Ficheiro > Abrir** e depois em **Procurar**, ou num nome em **Afixados** ou **Apresentações Recentes**.

O ficheiro abre-se nesta janela se contiver uma apresentação nova que ninguém tocou. Caso contrário, abre-se numa janela própria.

Clique com o botão direito num nome da lista para **Afixar na lista** (fica no topo), **Desafixar da lista** ou **Remover da lista**. Um nome a cinzento significa que o JeffDeck não consegue encontrar esse ficheiro neste momento.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista Recentes.

- **Abrir Só de Leitura** abre a apresentação apenas para leitura. Pode ver, percorrer os diapositivos e copiar, mas não pode alterar nada. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffDeck pede um novo nome, para que o ficheiro aberto não seja alterado.
- **Abrir como Cópia** abre os diapositivos como uma nova apresentação sem ficheiro próprio. **Guardar** pede onde a colocar.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo que a janela em que está esteja vazia.
- **Abrir e Reparar** destina-se a um ficheiro .pptx danificado, por exemplo um que foi cortado ao ser copiado. O JeffDeck lê todas as partes que consegue, mantém uma parte que parou a meio até ao ponto onde parou, e deixa de fora uma parte que não consegue ler. Os diapositivos reparados abrem-se como uma cópia chamada "Reparado" seguido do nome, e uma mensagem indica o que foi deixado de fora. O ficheiro danificado não é alterado.

## Ficheiros com palavra-passe

Um ficheiro **.pptx** que pede uma palavra-passe abre-se normalmente: o JeffDeck pede a palavra-passe, e pode tentar de novo se não estiver correta, ou clicar em **Cancelar**. O ficheiro continua protegido quando o guarda. Para alterar ou remover a palavra-passe, use **Ficheiro > Informações > Cifrar com Palavra-passe** (consulte O ecrã Ficheiro). Um ficheiro **.odp** ainda não pode conter uma palavra-passe.

## Tipos de letra no ficheiro

Um tipo de letra é a forma das letras. Nem todos os computadores têm todos os tipos de letra. Se um computador não tiver um tipo de letra que os seus diapositivos usam, desenha as letras noutro tipo de letra. Os diapositivos podem então ter um aspeto diferente.

O JeffDeck pode colocar os tipos de letra usados pelos seus diapositivos dentro do ficheiro. Assim, os diapositivos têm o mesmo aspeto em qualquer computador. Isto chama-se incorporação. O JeffDeck faz isto sempre que guarda. Para desativar, clique em **Ficheiro > Opções**, abra o separador **Guardar** e desmarque **Incorporar tipos de letra no ficheiro**.

- O JeffDeck coloca apenas os tipos de letra que vêm com ele, como Fraunces, Lato e Open Sans. Outros computadores não os têm. Nunca coloca um tipo de letra que todos os computadores têm, como Calibri ou Arial. Deixa de fora um tipo de letra cuja licença indica que não pode ser copiado para um ficheiro.
- O JeffDeck coloca cada estilo que os diapositivos usam: normal, negrito, itálico e negrito itálico. Se os diapositivos usarem apenas o estilo normal, só esse é incluído.
- O JeffDeck coloca todos os carateres de cada tipo de letra, e não apenas os que usou. Assim, quem abrir o ficheiro pode continuar a alterar as palavras.
- O ficheiro fica maior. Cada estilo de um tipo de letra acrescenta cerca de 40 a 700 KB (um KB são mil bytes). O ficheiro é comprimido ao ser guardado, por isso cresce cerca de metade disso.

Ao abrir um ficheiro que tem tipos de letra incorporados, o JeffDeck usa-os enquanto a apresentação estiver aberta. Os diapositivos têm o aspeto que tinham onde foram criados. O JeffDeck não instala os tipos de letra no seu computador, nem os copia para lado nenhum. Pertencem apenas a essa apresentação: não aparecem nas suas listas de tipos de letra, e nenhuma outra apresentação os usa. Desaparecem quando fechar a apresentação. Se o computador já tiver um tipo de letra, o JeffDeck usa a sua própria cópia.

O JeffDeck lê tipos de letra de ficheiros .pptx criados por outros programas de apresentação, mesmo quando esses programas os comprimiram. Também lê tipos de letra de ficheiros .odp.

Um ficheiro pode conter um tipo de letra que o JeffDeck não tem. Ao guardar, o JeffDeck volta a colocar esse tipo de letra no ficheiro, desde que os diapositivos ainda o usem. Faz isto, esteja ou não ativada a opção **Incorporar tipos de letra no ficheiro**. Se um tipo de letra de um ficheiro estiver danificado ou não puder ser lido, a mensagem que vê ao abrir o ficheiro indica-o.

## Tipos de ficheiros que o JeffDeck lê

- **.pptx, .potx e .ppsx** são um formato em três versões: apresentação, modelo e apresentação de diapositivos. O JeffDeck lê e escreve os três.
- **.ppt, .pps e .pot** são o tipo mais antigo. O JeffDeck lê-os, mas não consegue escrevê-los, por isso **Guardar** pede um novo nome e guarda um .pptx (um .ppsx para um .pps). Nunca substitui o original.
- **.odp e .otp** são apresentações de documento aberto e os respetivos modelos. O JeffDeck lê e escreve ambos. Um .odp aberto aqui é guardado como .odp. Um .otp é um modelo, por isso **Guardar** pede um novo nome e sugere um .odp.

Se parte de um ficheiro não puder ser lida, o JeffDeck abre o resto e indica o que deixou de fora. Se o ficheiro inteiro não puder ser lido, diz o motivo.

## Guardar como ficheiro .odp

Um ficheiro .odp abre-se no LibreOffice Impress com os seus diapositivos, modelos globais, imagens, notas e comentários, e com o aspeto que o JeffDeck mostra. Contém animações, transições de diapositivos, apresentações personalizadas e as definições da apresentação de diapositivos. O tipo de letra de cada palavra é guardado pelo nome, por isso um computador que não tenha o tipo de letra mostra outro.

O próprio JeffDeck ainda não lê de volta, a partir de um ficheiro .odp, as animações, comentários, apresentações personalizadas nem as definições da apresentação. Estão no ficheiro e o LibreOffice reproduz-nos, mas quando abre o ficheiro .odp no JeffDeck, este indica-os como deixados de fora. Mantenha o seu ficheiro .pptx se quiser continuar a editá-los aqui.

Um ficheiro .odp não tem lugar para algumas coisas. O JeffDeck indica-as depois de guardar, na mensagem na parte inferior da janela:

- **Gráficos, diagramas e equações** são guardados como imagens. Pode vê-los, mas não pode editar os seus números ou partes no ficheiro .odp.
- **Secções** não são guardadas.
- **Biséis e sombras interiores** são deixados de fora, e o mesmo acontece com brilhos e reflexos em texto. Uma sombra em texto é mantida. Um reflexo numa forma, uma linha com gradação, uma linha de dois ou três traços e o tamanho de uma sombra são mantidos de uma forma que só o JeffDeck lê. Outro programa mostra a forma sem o reflexo, a linha na sua primeira cor, um traço simples e a sombra com o tamanho do objeto.
- **Efeitos de animação** que começam ao clicar num objeto, e efeitos que alteram uma cor, são deixados de fora. Um clique num filme, para o pausar ou reproduzir de novo, é mantido.
- **Um filme ou som ligado** a partir de outro ficheiro, e não guardado na apresentação, mantém o seu endereço e a sua imagem no ficheiro .odp. O LibreOffice reproduz-no se conseguir encontrar o ficheiro. Os filmes e sons que estão na apresentação são guardados no ficheiro .odp e reproduzem-se.

Mantenha também o seu ficheiro .pptx, se precisar destas coisas de volta.

## O que é mantido mas não mostrado

Algumas partes de um ficheiro são coisas que o JeffDeck ainda não consegue mostrar nem alterar. Mantém-nas com os respetivos diapositivos e volta a escrevê-las ao guardar, para que não se percam. Incluem animações, que são reproduzidas na apresentação, filmes e sons, que ainda não são reproduzidos, e um efeito de transição para o qual o JeffDeck não tem forma, que é reproduzido como um desvanecimento. Os gráficos e diagramas são mostrados como foram guardados. Consulte [Bem-vindo ao JeffDeck](start).

## Ver também

- [Recuperar trabalho não guardado](recover)
- [O ecrã Ficheiro](file-screen)
- [Reutilizar diapositivos de outra apresentação](reuse)
- [Imprimir, PDF e imagens dos diapositivos](printing)
