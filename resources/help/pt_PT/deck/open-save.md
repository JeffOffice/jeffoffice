# Abrir e guardar apresentações
<!-- keywords: macros, com macros, pptm, potm, ppsm, abrir, guardar, guardar como, otp, libreoffice, deixado de fora, fechar, nova, ficheiros recentes, afixados, tipos de ficheiro, pptx, potx, ppsx, ppt, odp, modelo, apresentação, mudar o nome, Ctrl+S, cópia de segurança, formatos de ficheiro, ficheiros antigos, documento aberto, mantido, não mostrado, tipos de letra, incorporar tipos de letra, tipos de letra incorporados, tipo de letra em falta, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, meus modelos, guardar como modelo, reparar, abrir e reparar, danificado -->

O JeffDeck guarda o seu trabalho em ficheiros .pptx e em ficheiros .odp, e abre vários tipos de ficheiros de apresentação.

## Iniciar uma nova apresentação

Prima Ctrl+N. Abre-se uma nova janela com uma apresentação em branco. Ou clique em **Ficheiro > Novo** e depois em **Apresentação em Branco**. Se a janela em que está contiver uma apresentação nova que ninguém tocou, a apresentação em branco substitui-a. Caso contrário, abre-se numa janela própria.

## Guardar

Prima Ctrl+S, ou clique em **Ficheiro > Guardar**. Da primeira vez, o JeffDeck pede um nome e um local. Depois disso, Guardar substitui o ficheiro pelo seu trabalho mais recente. Uma mensagem na parte inferior diz "Guardado" e o nome do ficheiro.

Um asterisco (*) depois do nome na barra de título indica que tem alterações não guardadas. Guarde com frequência. Se o JeffDeck parar inesperadamente, consulte [Recuperar trabalho não guardado](recover).

Se uma mensagem disser que alguns objetos "não podem ser guardados neste tipo de ficheiro ainda", o ficheiro é guardado, mas o JeffDeck não tinha forma de escrever esses objetos. Guarde também o ficheiro original. Ao guardar um ficheiro .odp, a mensagem indica o que esse tipo de ficheiro não consegue conter. A secção "Guardar como um ficheiro .odp", abaixo, explica o que não pode ser guardado.

## Guardar com um novo nome ou tipo

Clique em **Ficheiro > Guardar Como**, ou prima F12. Escreva um nome, escolha o tipo e clique em **Guardar**. As opções são:

- **Apresentação (*.pptx)** é a escolha normal.
- **Modelo (*.potx)** guarda os diapositivos como padrão para iniciar novas apresentações.
- **Apresentação de diapositivos (*.ppsx)** guarda uma apresentação para ser reproduzida.
- **Apresentação com Macros (*.pptm)**, **Modelo com Macros (*.potm)** e **Apresentação de Diapositivos com Macros (*.ppsm)** são os mesmos três tipos para uma apresentação que contenha **macros**. Uma macro é um pequeno programa que acompanha um ficheiro. O JeffDeck nunca executa macros, e nunca as lê nem as mostra. Se abrir um destes ficheiros e o guardar com o mesmo tipo, as macros são mantidas tal como estavam. Se o guardar como .pptx, .potx ou .ppsx, as macros são deixadas de fora, e o JeffDeck avisa-o. Uma apresentação sem macros também pode ser guardada com estes tipos, mas não tem macros para manter. Se as macros de um ficheiro forem demasiado grandes para serem mantidas (mais de 32 MB numa parte, ou 64 MB no total), o JeffDeck deixa-as de fora e avisa-o quando abrir o ficheiro.
- **Apresentação OpenDocument (*.odp)** guarda os diapositivos no tipo de ficheiro usado pelo LibreOffice e por outros programas gratuitos.
- **Modelo de Apresentação OpenDocument (*.otp)** é a forma de modelo desse tipo.

Se não escrever a extensão, o JeffDeck acrescenta a que corresponde ao tipo escolhido. Depois de guardar, a barra de título mostra o novo nome, e continua a trabalhar no novo ficheiro.

Uma apresentação aberta a partir de um modelo é guardada como nova apresentação: **Guardar** pede um nome, para que o modelo se mantenha como estava. O mesmo acontece com uma apresentação aberta a partir de um ficheiro .ppt, .pps ou .pot mais antigo. Uma apresentação aberta a partir de um ficheiro .odp é guardada novamente como ficheiro .odp, no mesmo ficheiro.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia** e escolha um nome. O JeffDeck grava a apresentação nesse ficheiro, e continua a trabalhar no que tinha. O nome e as alterações não guardadas mantêm-se como estavam.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, no separador **Geral**, ative **Criar sempre cópia de segurança**. Cada vez que guardar, o JeffDeck copia primeiro o ficheiro antigo para outro chamado "Cópia de segurança de" seguido do nome do ficheiro, na mesma pasta. Se a cópia não puder ser criada, a aplicação pergunta se deve guardar sem ela. A resposta é Não, a menos que diga Sim, e o ficheiro mantém-se como estava.

## Guardar como modelo

Um modelo é uma apresentação que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo (um ficheiro **.potx**) é guardado em **Os Meus Modelos**, e a sua apresentação não é alterada. Para iniciar uma nova apresentação a partir dele, clique em **Ficheiro > Novo** e clique no modelo em **Os Meus Modelos**. A nova apresentação ainda não tem ficheiro próprio, por isso o modelo não é alterado. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Abrir

Prima Ctrl+O para escolher logo um ficheiro. Ou clique em **Ficheiro > Abrir** e depois em **Procurar**, ou num nome em **Afixados** ou **Apresentações Recentes**.

O ficheiro abre-se nesta janela se contiver uma apresentação nova que ninguém tocou. Caso contrário, abre-se numa janela própria.

Clique com o botão direito num nome da lista para **Afixar na lista** (fica no topo), **Desafixar da lista** ou **Remover da lista**. Um nome a cinzento significa que o JeffDeck não consegue encontrar esse ficheiro neste momento.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes.

- **Abrir Só para Leitura** abre a apresentação apenas para leitura. Pode ver, percorrer os diapositivos e copiar, mas não pode alterar nada. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffDeck pede um novo nome, para que o ficheiro que abriu não seja alterado.
- **Abrir como Cópia** abre os diapositivos como uma nova apresentação sem ficheiro próprio. **Guardar** pede onde a colocar.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo quando a janela em que está está vazia.
- **Abrir e Reparar** destina-se a um ficheiro .pptx ou .ppt danificado, por exemplo um que ficou truncado ao ser copiado. O JeffDeck lê todas as partes que consegue, mantém uma parte que parou a meio até ao ponto onde parou, e deixa de fora uma parte que não consegue ler. Num ficheiro .ppt antigo, encontra os diapositivos pelo conteúdo que têm quando as listas que indicam onde estão guardados estão danificadas ou perdidas. Se a lista que indica a ordem dos diapositivos se perder, os diapositivos aparecem na ordem em que foram guardados, que pode não ser a ordem original. Um ficheiro com palavra-passe não pode ser reparado. Os diapositivos reparados abrem-se como uma cópia chamada "Reparado" seguido do nome, e uma mensagem indica o que foi deixado de fora. O ficheiro danificado não é alterado.

## Ficheiros com palavra-passe

Um ficheiro **.pptx** ou **.odp** que pede uma palavra-passe abre-se normalmente: o JeffDeck pede a palavra-passe, e pode tentar novamente se não estiver correta, ou clicar em **Cancelar**. O ficheiro continua protegido quando o guarda. Para alterar ou remover a palavra-passe, use **Ficheiro > Informações > Cifrar com Palavra-passe** (consulte O ecrã Ficheiro). Uma cópia guardada como ficheiro .odp também mantém a palavra-passe.

## Tipos de letra no ficheiro

Um tipo de letra é o desenho das letras. Nem todos os computadores têm todos os tipos de letra. Se um computador não tiver um tipo de letra que os seus diapositivos usam, desenha as letras noutro tipo de letra. Nesse caso, os diapositivos podem ter um aspeto diferente.

O JeffDeck pode colocar os tipos de letra usados nos diapositivos dentro do próprio ficheiro. Assim, os diapositivos têm o mesmo aspeto em qualquer computador. Chama-se a isto incorporar. O JeffDeck faz isto cada vez que guarda. Para desativar, clique em **Ficheiro > Opções**, abra o separador **Guardar** e desmarque **Incorporar tipos de letra no ficheiro**.

- O JeffDeck incorpora apenas os tipos de letra que vêm com ele, como Fraunces, Lato e Open Sans. Outros computadores não os têm. Nunca incorpora um tipo de letra que todos os computadores têm, como Calibri ou Arial. Deixa de fora um tipo de letra cuja licença indique que não pode ser copiado para um ficheiro.
- O JeffDeck incorpora cada estilo que os diapositivos usam: normal, negrito, itálico e negrito itálico. Se os diapositivos usarem apenas o estilo normal, só esse é incorporado.
- O JeffDeck incorpora todas as letras de cada tipo de letra, e não apenas as que usou. Assim, quem abrir o ficheiro pode continuar a alterar as palavras.
- O ficheiro fica maior. Cada estilo de um tipo de letra acrescenta cerca de 40 a 700 KB (um KB é mil bytes). O ficheiro é comprimido quando é guardado, por isso cresce cerca de metade desse valor.

Quando abre um ficheiro que tem tipos de letra incorporados, o JeffDeck usa-os enquanto a apresentação estiver aberta. Os diapositivos têm o aspeto que tinham onde foram criados. O JeffDeck não instala os tipos de letra no seu computador, e não os copia para lado nenhum. Pertencem apenas a essa apresentação: não aparecem nas suas listas de tipos de letra, e nenhuma outra apresentação os usa. Desaparecem quando fecha a apresentação. Se o seu computador já tiver um tipo de letra, o JeffDeck usa a sua própria cópia.

O JeffDeck lê tipos de letra de ficheiros .pptx criados por outros programas de apresentação, mesmo quando esses programas os comprimiram. Também lê tipos de letra de ficheiros .odp.

Um ficheiro pode conter um tipo de letra que o JeffDeck não tem. Ao guardar, o JeffDeck volta a colocar esse tipo de letra no ficheiro, desde que os diapositivos ainda o usem. Faz isto quer **Incorporar tipos de letra no ficheiro** esteja ativado, quer não. Se um tipo de letra de um ficheiro estiver danificado ou não puder ser lido, a mensagem que vê ao abrir o ficheiro indica isso.

## Tipos de ficheiros que o JeffDeck lê

- **.pptx, .potx e .ppsx** são um formato em três formas: apresentação, modelo e apresentação de diapositivos. O JeffDeck lê e escreve as três.
- **.ppt, .pps e .pot** são o tipo mais antigo. O JeffDeck lê-os mas não os consegue escrever, por isso **Guardar** pede um novo nome e guarda um .pptx (um .ppsx, no caso de um .pps). Nunca substitui o original.
- **.odp e .otp** são apresentações em formato de documento aberto e os respetivos modelos. O JeffDeck lê e escreve ambos. Um .odp aberto aqui é guardado como .odp. Um .otp é um modelo, por isso **Guardar** pede um novo nome e sugere um .odp.

Se parte de um ficheiro não puder ser lida, o JeffDeck abre o resto e indica o que deixou de fora. Se o ficheiro inteiro não puder ser lido, diz porquê.

## Guardar como um ficheiro .odp

Um ficheiro .odp abre-se no LibreOffice Impress com os seus diapositivos, modelos globais, imagens, notas e comentários, e com o aspeto que o JeffDeck mostra. Contém animações, transições de diapositivos, apresentações personalizadas e as definições da apresentação de diapositivos. O tipo de letra de cada palavra é guardado pelo nome, por isso um computador que não tenha o tipo de letra mostra outro.

O JeffDeck ainda não lê de volta, a partir de um ficheiro .odp, as animações, os comentários, as apresentações personalizadas nem as definições da apresentação. Estão no ficheiro, e o LibreOffice reproduz-nos, mas quando abre o ficheiro .odp no JeffDeck, este indica-os como deixados de fora. Mantenha o seu ficheiro .pptx se quiser continuar a editá-los aqui.

Um ficheiro .odp não tem lugar para algumas coisas. O JeffDeck indica-as depois de guardar, na mensagem na parte inferior da janela:

- **Gráficos, diagramas e equações** são guardados como imagens. Pode vê-los, mas não pode editar os seus números ou partes no ficheiro .odp.
- **Secções** não são guardadas.
- **Biséis e sombras interiores** são deixados de fora, o mesmo acontecendo com brilhos e reflexos em texto. Uma sombra em texto é mantida. Um reflexo numa forma, uma linha com gradação, uma linha de dois ou três traços e o tamanho de uma sombra são mantidos de uma forma que só o JeffDeck lê. Outros programas mostram a forma sem o reflexo, a linha na sua primeira cor, um único traço e a sombra com o tamanho do objeto.
- **Efeitos de animação** que começam ao clicar num objeto, e efeitos que alteram uma cor, são deixados de fora. Um clique num vídeo, para o pausar ou reproduzir de novo, é mantido.
- **Um vídeo ou som ligado** a partir de outro ficheiro, e não guardado na apresentação, mantém o seu endereço e a sua imagem no ficheiro .odp. O LibreOffice reproduz-no se conseguir encontrar o ficheiro. Vídeos e sons que estão na apresentação são guardados no ficheiro .odp e reproduzem-se.

Mantenha também o seu ficheiro .pptx, se precisar destes elementos de volta.

## O que é mantido mas não mostrado

Algumas partes de um ficheiro são coisas que o JeffDeck ainda não consegue mostrar ou alterar. Mantém-nas com os respetivos diapositivos e escreve-as de volta quando guarda, para que não se percam. Incluem animações, que são reproduzidas na apresentação de diapositivos, vídeos e sons, que ainda não são reproduzidos, e um efeito de transição para o qual o JeffDeck não tem forma, que é reproduzido como desvanecimento. Os gráficos e diagramas são mostrados como foram guardados. Consulte [Bem-vindo ao JeffDeck](start).

## Relacionados

- [Recuperar trabalho não guardado](recover)
- [O ecrã Ficheiro](file-screen)
- [Reutilizar diapositivos de outra apresentação](reuse)
- [Imprimir, PDF e imagens de diapositivos](printing)
