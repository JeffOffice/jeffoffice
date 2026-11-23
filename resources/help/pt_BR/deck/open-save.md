# Abrir e salvar apresentações
<!-- keywords: macros, com macros, pptm, potm, ppsm, abrir, salvar, salvar como, otp, libreoffice, deixar de fora, fechar, nova, arquivos recentes, fixados, tipos de arquivo, pptx, potx, ppsx, ppt, odp, modelo, apresentação de slides, renomear, Ctrl+S, backup, cópia de segurança, formatos de arquivo, arquivos antigos, documento aberto, mantidos, não mostrados, fontes, inserir fontes, fontes inseridas, fonte ausente, salvar uma cópia, somente leitura, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, reparar, abrir e reparar, danificado -->

O JeffDeck salva o seu trabalho em arquivos .pptx e em arquivos .odp, e abre vários tipos de arquivos de apresentação.

## Iniciar uma nova apresentação

Pressione Ctrl+N. Uma nova janela abre com uma apresentação em branco. Ou clique em **Arquivo > Novo** e depois em **Apresentação em Branco**. Se a janela em que você está contém uma nova apresentação que ninguém tocou, a apresentação em branco ocupa o seu lugar. Caso contrário, ela abre em uma janela própria.

## Salvar

Pressione Ctrl+S, ou clique em **Arquivo > Salvar**. Na primeira vez, o JeffDeck pede um nome e um local. Depois disso, Salvar substitui o arquivo pelo seu trabalho mais recente. Uma mensagem na parte inferior diz "Salvo" e o nome do arquivo.

Um asterisco (*) depois do nome na barra de título significa que há alterações que você ainda não salvou. Salve com frequência. Se o JeffDeck parar de repente, veja [Recuperar trabalho não salvo](recover).

Se uma mensagem disser que alguns objetos "não podem ser salvos neste tipo de arquivo ainda", o arquivo é salvo, mas o JeffDeck não tinha como gravar esses objetos. Mantenha também o arquivo original. Ao salvar um arquivo .odp, a mensagem lista o que esse tipo de arquivo não consegue manter. A seção "Salvar como um arquivo .odp" abaixo explica o que ele não comporta.

## Salvar com um novo nome ou tipo

Clique em **Arquivo > Salvar Como**, ou pressione F12. Digite um nome, escolha o tipo e clique em **Salvar**. As opções são:

- **Apresentação (*.pptx)** é a escolha normal.
- **Modelo (*.potx)** salva os slides como um padrão para começar novas apresentações.
- **Apresentação de slides (*.ppsx)** salva uma apresentação para ser reproduzida.
- **Apresentação com Macros (*.pptm)**, **Modelo com Macros (*.potm)** e **Apresentação de Slides com Macros (*.ppsm)** são os mesmos três tipos para uma apresentação que contém **macros**. Uma macro é um pequeno programa que acompanha um arquivo. O JeffDeck nunca executa macros, e nunca as lê nem as mostra. Se você abrir um desses arquivos e o salvar no mesmo tipo, as macros são mantidas exatamente como estavam. Se você salvar como .pptx, .potx ou .ppsx, as macros são deixadas de fora, e o JeffDeck avisa você. Uma apresentação sem macros também pode ser salva nesses tipos, mas não há macros a manter. Se as macros de um arquivo forem grandes demais para serem mantidas (mais de 32 MB em uma parte, ou 64 MB no total), o JeffDeck as deixa de fora e avisa quando você abre o arquivo.
- **Apresentação OpenDocument (*.odp)** salva os slides no tipo de arquivo usado pelo LibreOffice e outros programas gratuitos.
- **Modelo de Apresentação OpenDocument (*.otp)** é a forma de modelo desse tipo.

Se você não digitar a extensão, o JeffDeck adiciona a que corresponde ao tipo. Depois de salvar, a barra de título mostra o novo nome, e você continua trabalhando no novo arquivo.

Uma apresentação aberta a partir de um modelo é salva como uma nova apresentação: **Salvar** pede um nome, para que o modelo permaneça como estava. O mesmo vale para uma apresentação aberta a partir de um arquivo .ppt, .pps ou .pot antigo. Uma apresentação aberta a partir de um arquivo .odp é salva de novo como .odp, sobre o mesmo arquivo.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia** e escolha um nome. O JeffDeck grava a apresentação nesse arquivo, e você continua trabalhando no que estava aberto. O nome e as alterações não salvas permanecem como estavam.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na guia **Geral**, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffDeck primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome dele, na mesma pasta. Se a cópia não puder ser feita, o programa pergunta se deve salvar sem ela. A resposta é Não, a menos que você diga Sim, e o arquivo permanece como estava.

## Salvar como modelo

Um modelo é uma apresentação que você usa como ponto de partida. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo (um arquivo **.potx**) vai para **Meus Modelos**, e a sua apresentação não é alterada. Para começar uma nova apresentação a partir dele, clique em **Arquivo > Novo** e clique no modelo em **Meus Modelos**. A nova apresentação ainda não tem arquivo próprio, portanto o modelo não é alterado. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Meus Modelos**.

## Abrir

Pressione Ctrl+O para escolher um arquivo imediatamente. Ou clique em **Arquivo > Abrir** e depois em **Procurar**, ou em um nome em **Fixados** ou **Apresentações Recentes**.

O arquivo abre nesta janela se ela contiver uma nova apresentação que ninguém tocou. Caso contrário, ele abre em uma janela própria.

Clique com o botão direito em um nome da lista para **Fixar na lista** (ele permanece no topo), **Desafixar da lista** ou **Remover da lista**. Um nome em cinza significa que o JeffDeck não consegue encontrar esse arquivo agora.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito em um nome da lista de Recentes.

- **Abrir como Somente Leitura** abre a apresentação apenas para leitura. Você pode consultar, percorrer os slides e copiar, mas não pode alterar nada. A barra de título mostra "Somente Leitura". Se você pressionar **Salvar**, o JeffDeck pede um novo nome, para que o arquivo aberto não seja alterado.
- **Abrir como Cópia** abre os slides como uma nova apresentação, sem arquivo próprio. **Salvar** pergunta onde guardá-la.
- **Abrir em Nova Janela** abre o arquivo em uma janela própria, mesmo quando a janela em que você está está vazia.
- **Abrir e Reparar** serve para um arquivo .pptx ou .ppt danificado, por exemplo um que foi cortado no meio ao ser copiado. O JeffDeck recupera o máximo que consegue: lê todas as partes que pode, mantém uma parte que foi interrompida até o ponto em que parou e deixa de fora uma parte que não consegue ler. Em um arquivo .ppt antigo, ele encontra os slides pelo conteúdo deles quando as listas que indicam onde estão guardados estão danificadas ou perdidas. Quando as listas que dizem como as partes do arquivo se encaixam estão danificadas, o programa considera que cada parte segue a anterior e mostra o que encontra ali. Essa parte pode pertencer a uma cópia mais antiga da apresentação ou a outra parte do arquivo, por isso a mensagem indica o slide onde ela começa e pede que você a compare com um backup, se tiver um. Se a suposição vier antes do primeiro slide, a mensagem diz que a apresentação inteira foi remontada. Se as imagens não puderam ser totalmente recuperadas, a mensagem informa isso. Se a lista que dá a ordem dos slides estiver perdida, os slides entram na ordem em que foram guardados, que pode não ser a ordem original. Um arquivo que tem senha não pode ser reparado. Os slides reparados abrem como uma cópia chamada "Repaired" seguido do nome, e uma mensagem informa o que foi feito. O arquivo danificado não é alterado.

## Arquivos com senha

Um arquivo **.pptx** ou **.odp** que pede senha abre normalmente: o JeffDeck pede a senha, e você pode tentar de novo se ela não estiver correta, ou clicar em **Cancelar**. O arquivo continua protegido quando você o salva. Para alterar ou remover a senha, use **Arquivo > Informações > Criptografar com Senha** (veja A tela Arquivo). Uma cópia salva como arquivo .odp mantém a senha também.

## Fontes no arquivo

Uma fonte é o desenho das letras. Nem todo computador tem todas as fontes. Se um computador não tiver uma fonte que os seus slides usam, ele desenha as letras em outra fonte. Então os slides podem ficar diferentes.

O JeffDeck pode colocar dentro do arquivo as fontes que os seus slides usam. Assim, os slides ficam iguais em qualquer computador. Isso se chama inserir. O JeffDeck faz isso a cada vez que você salva. Para desativar, clique em **Arquivo > Opções**, abra a guia **Salvar** e desmarque **Inserir fontes no arquivo**.

- O JeffDeck insere apenas as fontes que vêm com ele, como Fraunces, Lato e Open Sans. Outros computadores não têm essas. Ele nunca insere uma fonte que todo computador tem, como Calibri ou Arial. Ele deixa de fora uma fonte cuja licença diz que ela não pode ser copiada para um arquivo.
- O JeffDeck insere cada estilo que os slides usam: normal, negrito, itálico e negrito itálico. Se os slides usam somente o estilo normal, só esse entra.
- O JeffDeck insere todas as letras de cada fonte, não apenas as que você usou. Assim, quem abrir o arquivo ainda pode alterar as palavras.
- O arquivo fica maior. Cada estilo de uma fonte acrescenta cerca de 40 a 700 KB (um KB é mil bytes). O arquivo é compactado ao ser salvo, então ele cresce cerca de metade disso.

Ao abrir um arquivo que tem fontes dentro, o JeffDeck usa essas fontes enquanto a apresentação está aberta. Os slides ficam como estavam onde foram criados. O JeffDeck não instala as fontes no seu computador e não as copia para lugar algum. Elas pertencem apenas àquela apresentação: não aparecem nas suas listas de fontes, e nenhuma outra apresentação as usa. Elas desaparecem quando você fecha a apresentação. Se o seu computador já tiver uma fonte, o JeffDeck usa a sua própria cópia.

O JeffDeck lê fontes de arquivos .pptx feitos por outros programas de apresentação, mesmo quando o programa as compactou. Ele também lê fontes de arquivos .odp.

Um arquivo pode trazer uma fonte que o JeffDeck não tem. Ao salvar, o JeffDeck a devolve ao arquivo, desde que os slides ainda a usem. Isso acontece esteja ou não ativada a opção **Inserir fontes no arquivo**. Se uma fonte do arquivo estiver danificada ou não puder ser lida, a mensagem que você vê ao abrir o arquivo informa isso.

## Tipos de arquivo que o JeffDeck lê

- **.pptx, .potx e .ppsx** são um formato em três versões: apresentação, modelo e apresentação de slides. O JeffDeck lê e grava os três.
- **.ppt, .pps e .pot** são o tipo mais antigo. O JeffDeck lê esses arquivos, mas não consegue gravá-los, então **Salvar** pede um novo nome e salva um .pptx (um .ppsx, para um .pps). Ele nunca grava por cima do original.
- **.odp e .otp** são apresentações em formato de documento aberto e os modelos desse formato. O JeffDeck lê e grava os dois. Um .odp aberto aqui é salvo como .odp. Um .otp é um modelo, então **Salvar** pede um novo nome e sugere um .odp.

Se parte de um arquivo não puder ser lida, o JeffDeck abre o restante e lista o que deixou de fora. Se o arquivo inteiro não puder ser lido, ele informa o motivo.

## Salvar como um arquivo .odp

Um arquivo .odp abre no LibreOffice Impress com os seus slides, mestres, imagens, anotações e comentários, e com a aparência que o JeffDeck mostra. Ele guarda animações, transições de slides, apresentações personalizadas e as configurações da apresentação de slides também. A fonte de cada palavra é salva pelo nome, então um computador que não tenha a fonte mostra outra.

O JeffDeck ainda não lê de volta, a partir de um arquivo .odp, as animações, os comentários, as apresentações personalizadas nem as configurações da apresentação. Elas estão no arquivo, e o LibreOffice as reproduz, mas, ao abrir o arquivo .odp no JeffDeck, ele as lista como deixadas de fora. Mantenha o seu arquivo .pptx se quiser continuar editando essas coisas aqui.

Um arquivo .odp não tem lugar para algumas coisas. O JeffDeck as lista após o salvamento, na mensagem na parte inferior da janela:

- **Gráficos, diagramas e equações** são salvos como imagens. Você pode vê-los, mas não pode editar seus números ou partes no arquivo .odp.
- **Seções não são salvas.**
- **Biséis e sombras internas** são deixados de fora, assim como brilhos e reflexos em textos. Uma sombra em texto é mantida. Um reflexo em uma forma, uma linha com gradiente, uma linha de duas ou três espessuras e o tamanho de uma sombra são mantidos de um modo que só o JeffDeck entende. Outro programa mostra a forma sem o reflexo, a linha na sua primeira cor, um traço único e a sombra no tamanho do objeto.
- **Efeitos de animação** que começam ao clicar em um objeto, e efeitos que alteram uma cor, são deixados de fora. Um clique em um vídeo, para pausá-lo ou reproduzi-lo de novo, é mantido.
- **Um vídeo ou som vinculado** a partir de outro arquivo, e não guardado na apresentação, mantém seu endereço e sua imagem no arquivo .odp. O LibreOffice o reproduz se conseguir encontrar o arquivo. Vídeos e sons que estão na apresentação são guardados no arquivo .odp e são reproduzidos.

Mantenha também o seu arquivo .pptx, se precisar desses recursos de volta.

## O que é mantido, mas não mostrado

Algumas partes de um arquivo são coisas que o JeffDeck ainda não consegue mostrar nem alterar. Ele as mantém junto com os seus slides e as grava de volta ao salvar, para que não se percam. Entre elas estão as animações, que são reproduzidas na apresentação de slides; vídeos e sons, que ainda não são reproduzidos; e um efeito de transição para o qual o JeffDeck não tem forma, que é reproduzido como esmaecimento. Gráficos e diagramas aparecem como foram salvos. Veja [Bem-vindo ao JeffDeck](start).

## Relacionados

- [Recuperar trabalho não salvo](recover)
- [A tela Arquivo](file-screen)
- [Reutilizar slides de outra apresentação](reuse)
- [Imprimir, PDF e imagens dos slides](printing)
