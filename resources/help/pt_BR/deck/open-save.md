# Abrir e salvar apresentações
<!-- keywords: abrir, salvar, salvar como, otp, libreoffice, omitido, deixado de fora, fechar, novo, nova apresentação, arquivos recentes, fixado, tipos de arquivo, formatos, pptx, potx, ppsx, ppt, odp, modelo, apresentação de slides, renomear, Ctrl+S, backup, cópia de segurança, formatos de arquivo, arquivos antigos, open document, mantido, preservado, não exibido, fontes, inserir fontes, fontes inseridas, incorporar fontes, fonte ausente, fonte faltando -->

O JeffDeck salva seu trabalho em arquivos .pptx e .odp, e abre vários tipos de arquivos de apresentação.

## Iniciar uma nova apresentação

Pressione Ctrl+N. Uma nova janela é aberta com uma apresentação em branco. Ou clique em **Arquivo > Novo** e depois em **Apresentação em Branco**. Se a janela em que você está contém uma apresentação nova que ninguém alterou, a apresentação em branco toma o lugar dela. Caso contrário, ela é aberta em uma janela própria.

## Salvar

Pressione Ctrl+S ou clique em **Arquivo > Salvar**. Na primeira vez, o JeffDeck pede um nome e um local. Depois disso, Salvar substitui o arquivo pela versão mais recente do seu trabalho. Uma mensagem na parte inferior mostra "Salvo" e o nome do arquivo.

Um asterisco (*) depois do nome na barra de título indica que há alterações não salvas. Salve com frequência. Se o JeffDeck fechar inesperadamente, consulte [Recuperar trabalho não salvo](recover).

Se uma mensagem informar que alguns objetos "ainda não podem ser salvos neste tipo de arquivo", o arquivo foi salvo, mas o JeffDeck não teve como gravar esses objetos. Guarde também o arquivo original. Quando você salva um arquivo .odp, a mensagem lista o que esse tipo de arquivo não comporta. A seção "Salvar como arquivo .odp", mais abaixo, explica o que ele não comporta.

## Salvar com outro nome ou tipo

Clique em **Arquivo > Salvar como** ou pressione F12. Digite um nome, escolha o tipo e clique em **Salvar**. As opções são:

- **Apresentação (*.pptx)** é a opção normal.
- **Modelo (*.potx)** salva os slides como base para iniciar novas apresentações.
- **Apresentação de slides (*.ppsx)** salva uma apresentação para ser exibida.
- **Apresentação OpenDocument (*.odp)** salva os slides no tipo de arquivo usado pelo LibreOffice e por outros programas gratuitos.
- **Modelo de Apresentação OpenDocument (*.otp)** é a versão de modelo desse tipo.

Se você não digitar a extensão, o JeffDeck adiciona a que corresponde ao tipo. Depois de salvar, a barra de título mostra o novo nome, e você continua trabalhando no novo arquivo.

Uma apresentação aberta a partir de um modelo é salva como uma nova apresentação: **Salvar** pede um nome, para que o modelo continue como estava. O mesmo vale para uma apresentação aberta a partir de um arquivo antigo .ppt, .pps ou .pot. Uma apresentação aberta a partir de um arquivo .odp é salva novamente como .odp, no mesmo arquivo.

## Abrir

Pressione Ctrl+O para escolher um arquivo diretamente. Ou clique em **Arquivo > Abrir** e depois em **Procurar**, ou em um nome em **Fixado** ou **Apresentações Recentes**.

O arquivo é aberto nesta janela se ela contiver uma apresentação nova que ninguém alterou. Caso contrário, ele é aberto em uma janela própria.

Clique com o botão direito do mouse em um nome da lista para **Fixar na lista** (ele fica no topo), **Desafixar da lista** ou **Remover da lista**. Um nome em cinza indica que o JeffDeck não encontra mais esse arquivo.

## Fontes no arquivo

A fonte é o formato das letras. Nem todo computador tem todas as fontes. Se faltar em um computador uma fonte usada nos seus slides, ele desenha as letras com outra fonte. Assim, os slides podem ficar com aparência diferente.

O JeffDeck pode colocar dentro do arquivo as fontes usadas nos slides. Assim, os slides ficam iguais em qualquer computador. Isso se chama inserir fontes. O JeffDeck faz isso sempre que você salva. Para desativar, clique em **Arquivo > Opções**, abra a guia **Salvar** e desmarque **Inserir fontes no arquivo**.

- O JeffDeck insere apenas as fontes que vêm com ele, como Fraunces, Lato e Open Sans. Outros computadores não têm essas fontes. Ele nunca insere uma fonte que todo computador tem, como Calibri ou Arial. Ele deixa de fora as fontes cuja licença não permite copiá-las para um arquivo.
- O JeffDeck insere cada estilo usado nos slides: normal, negrito, itálico e negrito itálico. Se os slides usam apenas o estilo normal, só ele é inserido.
- O JeffDeck insere todas as letras de cada fonte, e não apenas as que você usou. Assim, quem abrir o arquivo ainda pode alterar o texto.
- O arquivo fica maior. Cada estilo de fonte acrescenta de 40 a 700 KB (um KB equivale a mil bytes). O arquivo é compactado ao ser salvo, então aumenta cerca da metade disso.

Quando você abre um arquivo com fontes inseridas, o JeffDeck as usa enquanto a apresentação estiver aberta. Os slides ficam com a mesma aparência de onde foram criados. O JeffDeck não instala as fontes no seu computador nem as copia para outro lugar. Elas pertencem somente àquela apresentação: não aparecem nas suas listas de fontes, e nenhuma outra apresentação as usa. Elas deixam de estar disponíveis quando você fecha a apresentação. Se o seu computador já tiver uma fonte, o JeffDeck usa a sua própria cópia.

O JeffDeck lê fontes de arquivos .pptx criados por outros programas de apresentação, mesmo quando o programa as compactou. Ele também lê fontes de arquivos .odp.

Um arquivo pode conter uma fonte que o JeffDeck não tem. Ao salvar, o JeffDeck coloca essa fonte de volta no arquivo, desde que os slides ainda a usem. Ele faz isso esteja a opção **Inserir fontes no arquivo** ativada ou não. Se uma fonte do arquivo estiver danificada ou não puder ser lida, a mensagem exibida ao abrir o arquivo informa isso.

## Tipos de arquivo que o JeffDeck lê

- **.pptx, .potx e .ppsx** são um mesmo formato em três versões: apresentação, modelo e apresentação de slides. O JeffDeck lê e grava os três.
- **.ppt, .pps e .pot** são o tipo mais antigo. O JeffDeck os lê, mas não consegue gravá-los; por isso, **Salvar** pede um novo nome e salva um .pptx (um .ppsx no caso de um .pps). Ele nunca grava por cima do original.
- **.odp e .otp** são apresentações OpenDocument e seus modelos. O JeffDeck lê e grava os dois. Um .odp aberto aqui é salvo como .odp. Um .otp é um modelo; por isso, **Salvar** pede um novo nome e sugere um .odp.

Se parte de um arquivo não puder ser lida, o JeffDeck abre o restante e lista o que deixou de fora. Se o arquivo inteiro não puder ser lido, ele informa o motivo.

## Salvar como arquivo .odp

Um arquivo .odp abre no LibreOffice Impress com seus slides, mestres, imagens, anotações e comentários, e com a aparência mostrada pelo JeffDeck. Ele também contém animações, transições de slide, apresentações personalizadas e as configurações da apresentação de slides. A fonte de cada palavra é salva pelo nome; assim, um computador que não tenha a fonte mostra outra no lugar.

O próprio JeffDeck ainda não lê de volta animações, comentários, apresentações personalizadas nem configurações da apresentação de um arquivo .odp. Eles estão no arquivo, e o LibreOffice os reproduz, mas quando você abre o arquivo .odp no JeffDeck, ele os lista como deixados de fora. Guarde o arquivo .pptx se quiser continuar a editá-los aqui.

Um arquivo .odp não tem lugar para algumas coisas. O JeffDeck as lista depois de salvar, na mensagem na parte inferior da janela:

- **Gráficos, diagramas SmartArt e equações** são salvos como imagens. Você pode vê-los, mas não pode editar seus números ou partes no arquivo .odp.
- **Seções** não são salvas.
- **Biséis e sombras internas** são deixados de fora, assim como brilho e reflexo em texto. A sombra em texto é mantida. Reflexo em uma forma, linha com gradiente, linha de dois ou três traços e o tamanho da sombra são mantidos de um modo que só o JeffDeck lê. Outro programa mostra a forma sem o reflexo, a linha na primeira cor, um traço único e a sombra no tamanho do objeto.
- **Efeitos de animação** que começam quando você clica em um objeto e efeitos que alteram uma cor são deixados de fora. O clique em um vídeo, para pausá-lo ou reproduzi-lo novamente, é mantido.
- **Um vídeo ou som vinculado** a outro arquivo, e não guardado na apresentação, mantém seu endereço e sua imagem no arquivo .odp. O LibreOffice o reproduz se encontrar o arquivo. Vídeos e sons que estão na apresentação são mantidos no arquivo .odp e são reproduzidos.

Guarde também o arquivo .pptx, se precisar recuperar esses itens.

## O que é mantido, mas não exibido

Algumas partes de um arquivo são itens que o JeffDeck ainda não consegue mostrar nem alterar. Ele os mantém com seus slides e os grava de volta ao salvar, para que não se percam. Isso inclui animações, que são reproduzidas na apresentação; vídeos e sons, que ainda não são reproduzidos; e efeitos de transição para os quais o JeffDeck não tem equivalente, que são reproduzidos como esmaecimento. Gráficos e diagramas SmartArt aparecem como foram salvos. Consulte [Bem-vindo ao JeffDeck](start).

## Relacionados

- [Recuperar trabalho não salvo](recover)
- [A tela Arquivo](file-screen)
- [Reutilizar slides de outra apresentação](reuse)
- [Impressão, PDF e imagens de slides](printing)
