# Abrir e salvar apresentações
<!-- keywords: abrir, salvar, salvar como, otp, libreoffice, deixar de fora, fechar, novo, arquivos recentes, fixados, tipos de arquivo, pptx, potx, ppsx, ppt, odp, modelo, apresentação de slides, mostrar, renomear, Ctrl+S, backup, cópia de segurança, formatos de arquivo, arquivos antigos, documento aberto, mantido, não mostrado, fontes, inserir fontes, fontes inseridas, fonte ausente, salvar uma cópia, somente leitura, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, reparar, abrir e reparar, danificado -->

O JeffDeck salva o seu trabalho em arquivos .pptx e em arquivos .odp, e abre vários tipos de arquivos de apresentação.

## Iniciar uma nova apresentação

Pressione Ctrl+N. Uma nova janela se abre com uma apresentação em branco. Ou clique em **Arquivo > Novo** e depois em **Apresentação em Branco**. Se a janela em que você está tiver uma apresentação nova que ninguém alterou, a apresentação em branco ocupa o lugar dela. Caso contrário, ela se abre em uma janela própria.

## Salvar

Pressione Ctrl+S, ou clique em **Arquivo > Salvar**. Na primeira vez, o JeffDeck pede um nome e um local. Depois disso, Salvar substitui o arquivo pelo seu trabalho mais recente. Uma mensagem na parte inferior diz "Salvo" e o nome do arquivo.

Um asterisco (*) depois do nome na barra de título significa que há alterações que você ainda não salvou. Salve com frequência. Se o JeffDeck parar de repente, consulte [Recuperar trabalho não salvo](recover).

Se uma mensagem disser que alguns objetos "não podem ser salvos neste tipo de arquivo ainda", o arquivo é salvo, mas o JeffDeck não tinha como gravar esses objetos. Mantenha também o arquivo original. Ao salvar um arquivo .odp, a mensagem lista o que esse tipo de arquivo não consegue guardar. A seção "Salvar como um arquivo .odp", abaixo, explica o que ele não guarda.

## Salvar com outro nome ou tipo

Clique em **Arquivo > Salvar Como**, ou pressione F12. Digite um nome, escolha o tipo e clique em **Salvar**. As opções são:

- **Apresentação (*.pptx)** é a escolha normal.
- **Modelo (*.potx)** salva os slides como um padrão para começar novas apresentações.
- **Apresentação de slides (*.ppsx)** salva a apresentação para ser exibida.
- **Apresentação OpenDocument (*.odp)** salva os slides no tipo de arquivo que o LibreOffice e outros programas gratuitos usam.
- **Modelo de Apresentação OpenDocument (*.otp)** é a forma de modelo desse tipo.

Se você não digitar a extensão, o JeffDeck adiciona a que corresponde ao tipo escolhido. Depois de salvar, a barra de título mostra o novo nome, e você continua trabalhando no novo arquivo.

Uma apresentação aberta de um modelo é salva como uma apresentação nova: **Salvar** pede um nome, para que o modelo permaneça como estava. O mesmo vale para uma apresentação aberta de um arquivo .ppt, .pps ou .pot mais antigo. Uma apresentação aberta de um arquivo .odp é salva novamente como arquivo .odp, no mesmo arquivo.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia** e escolha um nome. O JeffDeck grava a apresentação nesse arquivo, e você continua trabalhando na que tinha antes. O nome dela e as alterações não salvas permanecem como estavam.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na guia **Geral**, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffDeck primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome dele, na mesma pasta. Se a cópia não puder ser feita, o aplicativo pergunta se deve salvar sem ela. A resposta padrão é Não, a menos que você diga Sim, e o arquivo permanece como estava.

## Salvar como modelo

Um modelo é uma apresentação que você usa como ponto de partida. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo (um arquivo **.potx**) vai para **Meus Modelos**, e a sua apresentação não é alterada. Para começar uma nova apresentação a partir dele, clique em **Arquivo > Novo** e clique no modelo em **Meus Modelos**. A nova apresentação ainda não tem arquivo próprio, então o modelo não é alterado. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Meus Modelos**.

## Abrir

Pressione Ctrl+O para escolher um arquivo imediatamente. Ou clique em **Arquivo > Abrir** e depois em **Procurar**, ou em um nome em **Fixados** ou **Apresentações Recentes**.

O arquivo se abre nesta janela se ela tiver uma apresentação nova que ninguém alterou. Caso contrário, ele se abre em uma janela própria.

Clique com o botão direito em um nome da lista para **Fixar na lista** (ele fica no topo), **Desafixar da lista** ou **Remover da lista**. Um nome em cinza significa que o JeffDeck não encontra mais esse arquivo.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito em um nome da lista de Recentes.

- **Abrir como Somente Leitura** abre a apresentação apenas para leitura. Você pode ver, passar pelos slides e copiar, mas não pode alterar nada. A barra de título mostra "Somente Leitura". Se você pressionar **Salvar**, o JeffDeck pede um novo nome, para que o arquivo aberto não seja alterado.
- **Abrir como Cópia** abre os slides como uma nova apresentação, sem arquivo próprio. **Salvar** pede onde guardá-la.
- **Abrir em Nova Janela** abre o arquivo em uma janela própria, mesmo quando a janela em que você está estiver vazia.
- **Abrir e Reparar** serve para um arquivo .pptx danificado, por exemplo um que foi cortado ao ser copiado. O JeffDeck lê todas as partes que consegue, mantém uma parte interrompida até o ponto em que parou e deixa de fora uma parte que não consegue ler. Os slides reparados abrem como uma cópia chamada "Reparado" seguido do nome, e uma mensagem informa o que foi deixado de fora. O arquivo danificado não é alterado.

## Fontes no arquivo

Uma fonte é o desenho das letras. Nem todo computador tem todas as fontes. Se um computador não tiver uma fonte que os seus slides usam, ele desenha as letras em outra fonte. Assim, os slides podem ficar diferentes.

O JeffDeck pode colocar dentro do arquivo as fontes que os seus slides usam. Assim, os slides ficam iguais em qualquer computador. Isso se chama inserir fontes. O JeffDeck faz isso a cada vez que você salva. Para desativar, clique em **Arquivo > Opções**, abra a guia **Salvar** e desmarque **Inserir fontes no arquivo**.

- O JeffDeck insere apenas as fontes que vêm com ele, como Fraunces, Lato e Open Sans. Outros computadores não têm essas fontes. Ele nunca insere uma fonte que todo computador tem, como Calibri ou Arial. Deixa de fora uma fonte cuja licença diz que ela não pode ser copiada para um arquivo.
- O JeffDeck insere cada estilo que os slides usam: normal, negrito, itálico e negrito itálico. Se os slides usarem apenas o estilo normal, só esse é inserido.
- O JeffDeck insere todas as letras de cada fonte, e não apenas as que você usou. Assim, quem abrir o arquivo ainda pode alterar as palavras.
- O arquivo fica maior. Cada estilo de uma fonte acrescenta cerca de 40 a 700 KB (um KB é mil bytes). O arquivo é compactado ao ser salvo, então ele cresce cerca de metade desse valor.

Ao abrir um arquivo que tem fontes inseridas, o JeffDeck as usa enquanto a apresentação estiver aberta. Os slides ficam como estavam no lugar em que foram criados. O JeffDeck não instala as fontes no seu computador e não as copia para lugar nenhum. Elas pertencem apenas àquela apresentação: não aparecem nas suas listas de fontes, e nenhuma outra apresentação as usa. Elas desaparecem quando você fecha a apresentação. Se o seu computador já tiver uma fonte, o JeffDeck usa a sua própria cópia.

O JeffDeck lê fontes de arquivos .pptx feitos por outros programas de apresentação, mesmo quando o programa as compactou. Ele também lê fontes de arquivos .odp.

Um arquivo pode trazer uma fonte que o JeffDeck não tem. Ao salvar, o JeffDeck grava essa fonte de volta no arquivo, desde que os slides ainda a usem. Isso acontece esteja ou não ativada a opção **Inserir fontes no arquivo**. Se uma fonte do arquivo estiver danificada ou não puder ser lida, a mensagem que você vê ao abrir o arquivo informa isso.

## Tipos de arquivo que o JeffDeck lê

- **.pptx, .potx e .ppsx** são um formato em três formas: apresentação, modelo e apresentação de slides. O JeffDeck lê e grava os três.
- **.ppt, .pps e .pot** são o tipo mais antigo. O JeffDeck os lê, mas não consegue gravá-los, então **Salvar** pede um novo nome e salva um .pptx (um .ppsx, no caso de .pps). Ele nunca grava por cima do original.
- **.odp e .otp** são apresentações no formato OpenDocument e os respectivos modelos. O JeffDeck lê e grava os dois. Um .odp aberto aqui é salvo como .odp. Um .otp é um modelo, então **Salvar** pede um novo nome e sugere um .odp.

Se parte de um arquivo não puder ser lida, o JeffDeck abre o restante e lista o que deixou de fora. Se o arquivo inteiro não puder ser lido, ele informa o motivo.

## Salvar como um arquivo .odp

Um arquivo .odp abre no LibreOffice Impress com os seus slides, mestres, imagens, anotações e comentários, e com a aparência que o JeffDeck mostra. Ele guarda animações, transições de slides, apresentações personalizadas e as configurações da apresentação de slides também. A fonte de cada palavra é salva pelo nome, então um computador que não tenha a fonte mostra outra.

O JeffDeck ainda não lê de volta animações, comentários, apresentações personalizadas nem as configurações da apresentação de um arquivo .odp. Eles estão no arquivo, e o LibreOffice os reproduz, mas, quando você abre o arquivo .odp no JeffDeck, ele os lista como deixados de fora. Mantenha o seu arquivo .pptx se quiser continuar editando essas coisas aqui.

Um arquivo .odp não tem lugar para algumas coisas. O JeffDeck as lista após o salvamento, na mensagem na parte inferior da janela:

- **Gráficos, diagramas do SmartArt e equações** são salvos como imagens. Você pode vê-los, mas não pode editar seus números ou partes no arquivo .odp.
- **Seções** não são salvas.
- **Biséis e sombras internas** são deixados de fora, assim como brilhos e reflexos em textos. Uma sombra em texto é mantida. Um reflexo em uma forma, uma linha com gradiente, uma linha de dois ou três traços e o tamanho de uma sombra são mantidos de um modo que só o JeffDeck entende. Outro programa mostra a forma sem o reflexo, a linha na sua primeira cor, um traço único e a sombra no tamanho do objeto.
- **Efeitos de animação** que começam ao clicar em um objeto, e efeitos que mudam uma cor, são deixados de fora. Um clique em um vídeo, para pausá-lo ou reproduzi-lo de novo, é mantido.
- **Um vídeo ou som vinculado** de outro arquivo, e não guardado na apresentação, mantém o seu endereço e a sua imagem no arquivo .odp. O LibreOffice o reproduz se conseguir encontrar o arquivo. Vídeos e sons que estão na apresentação são mantidos no arquivo .odp e são reproduzidos.

Mantenha também o seu arquivo .pptx, se precisar recuperar essas coisas.

## O que é mantido, mas não mostrado

Algumas partes de um arquivo são coisas que o JeffDeck ainda não consegue mostrar nem alterar. Ele as mantém junto com os seus slides e as grava de volta ao salvar, para que não se percam. Elas incluem animações, que são reproduzidas na apresentação; vídeos e sons, que ainda não são reproduzidos; e um efeito de transição para o qual o JeffDeck não tem forma, que é reproduzido como esmaecer. Gráficos e diagramas do SmartArt são mostrados como foram salvos. Consulte [Bem-vindo ao JeffDeck](start).

## Relacionados

- [Recuperar trabalho não salvo](recover)
- [A tela Arquivo](file-screen)
- [Reutilizar slides de outra apresentação](reuse)
- [Imprimir, PDF e imagens dos slides](printing)
