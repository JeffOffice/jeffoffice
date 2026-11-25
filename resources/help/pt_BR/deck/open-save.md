# Abrir e salvar apresentações
<!-- keywords: macros, com macros, pptm, potm, ppsm, abrir, salvar, salvar como, otp, libreoffice, deixados de fora, fechar, nova, arquivos recentes, fixados, tipos de arquivo, pptx, potx, ppsx, ppt, odp, modelo, apresentação de slides, renomear, Ctrl+S, backup, cópia de segurança, formatos de arquivo, arquivos antigos, documento aberto, mantidos, não mostrados, fontes, inserir fontes, fontes inseridas, fonte ausente, salvar uma cópia, somente leitura, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, reparar, abrir e reparar, danificado -->

O JeffDeck salva o seu trabalho em arquivos .pptx e em arquivos .odp, e abre vários tipos de arquivos de apresentação.

## Iniciar uma nova apresentação

Pressione Ctrl+N. Uma nova janela se abre com uma apresentação em branco. Ou clique em **Arquivo > Novo** e depois em **Apresentação em Branco**. Se a janela em que você está tiver uma apresentação nova que ninguém alterou, a apresentação em branco ocupa o lugar dela. Caso contrário, ela se abre em uma janela própria.

## Salvar

Pressione Ctrl+S ou clique em **Arquivo > Salvar**. Na primeira vez, o JeffDeck pede um nome e um local. Depois disso, Salvar substitui o arquivo pelo seu trabalho mais recente. Uma mensagem na parte inferior diz "Salvo" e o nome do arquivo.

Um asterisco (*) depois do nome na barra de título indica que há alterações que você ainda não salvou. Salve com frequência. Se o JeffDeck parar de repente, veja [Recuperar trabalho não salvo](recover).

Se uma mensagem disser que alguns objetos "não podem ser salvos neste tipo de arquivo ainda", o arquivo é salvo, mas o JeffDeck não tinha como gravar esses objetos. Guarde também o arquivo original. Ao salvar um arquivo .odp, a mensagem lista o que esse tipo de arquivo não pode conter. A seção "Salvar como um arquivo .odp", mais abaixo, explica o que ele não comporta.

## Salvar com um novo nome ou tipo

Clique em **Arquivo > Salvar Como** ou pressione F12. Digite um nome, escolha o tipo e clique em **Salvar**. As opções são:

- **Apresentação (*.pptx)** é a escolha normal.
- **Modelo (*.potx)** salva os slides como um padrão para iniciar novas apresentações.
- **Apresentação de Slides (*.ppsx)** salva uma apresentação para ser reproduzida.
- **Apresentação com Macros (*.pptm)**, **Modelo com Macros (*.potm)** e **Apresentação de Slides com Macros (*.ppsm)** são os mesmos três tipos, para uma apresentação que contenha **macros**. Uma macro é um pequeno programa que acompanha um arquivo. O JeffDeck nunca executa macros e nunca as lê nem as mostra. Se você abrir um desses arquivos e salvá-lo com o mesmo tipo, as macros são mantidas exatamente como estavam. Se você salvá-lo como .pptx, .potx ou .ppsx, as macros são deixadas de fora, e o JeffDeck avisa você. Uma apresentação sem macros também pode ser salva com esses tipos, mas não há macros a manter. Se as macros de um arquivo forem grandes demais para serem mantidas (mais de 32 MB em uma parte, ou 64 MB no total), o JeffDeck as deixa de fora e avisa quando você o abre.
- **Apresentação OpenDocument (*.odp)** salva os slides no formato de arquivo usado pelo LibreOffice e por outros programas gratuitos.
- **Modelo de Apresentação OpenDocument (*.otp)** é a forma de modelo desse tipo.

Se você omitir a extensão, o JeffDeck acrescenta a que corresponde ao tipo. Depois de salvar, a barra de título mostra o novo nome, e você continua trabalhando no novo arquivo.

Uma apresentação aberta a partir de um modelo é salva como uma nova apresentação: **Salvar** pede um nome, para que o modelo permaneça como estava. O mesmo vale para uma apresentação aberta a partir de um arquivo .ppt, .pps ou .pot antigo. Uma apresentação aberta a partir de um arquivo .odp é salva novamente como arquivo .odp, sobre o mesmo arquivo.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia** e escolha um nome. O JeffDeck grava a apresentação nesse arquivo, e você continua trabalhando na que tinha. O nome dela e as alterações não salvas permanecem como estavam.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na guia **Geral**, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffDeck primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome dele, na mesma pasta. Se a cópia não puder ser feita, o aplicativo pergunta se deve salvar sem ela. A resposta é Não, a menos que você diga Sim, e o arquivo permanece como estava.

## Salvar como modelo

Um modelo é uma apresentação que você usa como ponto de partida. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo (um arquivo **.potx**) vai para **Meus Modelos**, e a sua apresentação não é alterada. Para iniciar uma nova apresentação a partir dele, clique em **Arquivo > Novo** e clique no modelo em **Meus Modelos**. A nova apresentação ainda não tem arquivo próprio, então o modelo não é alterado. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Meus Modelos**.

## Abrir

Pressione Ctrl+O para escolher um arquivo imediatamente. Ou clique em **Arquivo > Abrir** e depois em **Procurar**, ou em um nome em **Fixados** ou **Apresentações Recentes**.

O arquivo se abre nesta janela se ela tiver uma apresentação nova que ninguém alterou. Caso contrário, ele se abre em uma janela própria.

Clique com o botão direito em um nome da lista para **Fixar na lista** (ele fica no topo), **Desafixar da lista** ou **Remover da lista**. Um nome em cinza indica que o JeffDeck não consegue encontrar o arquivo agora.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito em um nome da lista de Recentes.

- **Abrir como Somente Leitura** abre a apresentação apenas para leitura. Você pode vê-la, percorrer os slides e copiar, mas não pode alterar nada. A barra de título mostra "Somente Leitura". Se você pressionar **Salvar**, o JeffDeck pede um novo nome, para que o arquivo aberto não seja alterado.
- **Abrir como Cópia** abre os slides como uma nova apresentação sem arquivo próprio. **Salvar** pergunta onde guardá-la.
- **Abrir em Nova Janela** abre o arquivo em uma janela própria, mesmo quando a janela em que você está está vazia.
- **Abrir e Reparar** serve para um arquivo .pptx ou .ppt danificado, por exemplo um que foi cortado durante uma cópia. O JeffDeck recupera o máximo que consegue: lê cada parte que pode ser lida, mantém uma parte interrompida até o ponto em que parou e deixa de fora uma parte que não consegue ler. Em um arquivo .ppt antigo, ele encontra os slides pelo conteúdo deles quando as listas que indicam onde estão guardados estiverem danificadas ou perdidas. Quando as listas que indicam como as partes do arquivo se encaixam estiverem danificadas, o programa pega cada parte para seguir a anterior e mostra o que encontra ali. Essa parte pode pertencer a uma cópia mais antiga da apresentação ou a outra parte do arquivo, por isso a mensagem indica o slide em que ela começa e pede que você a compare com um backup, se tiver um. Se a suposição vier antes do primeiro slide, a mensagem diz que a apresentação inteira foi reconstruída. Se as imagens não puderem ser totalmente recuperadas, a mensagem informa isso. Se a lista que define a ordem dos slides for perdida, os slides entram na ordem em que foram guardados, que pode não ser a ordem original. Um arquivo que tem senha não pode ser reparado. Os slides reparados se abrem como uma cópia chamada "Reparado" seguido do nome, e uma mensagem informa o que foi feito. O arquivo danificado não é alterado.

## Arquivos com senha

Um arquivo **.pptx** ou **.odp** que pede senha se abre normalmente: o JeffDeck pede a senha, e você pode tentar de novo se ela estiver errada, ou clicar em **Cancelar**. O arquivo continua protegido ao ser salvo. Para alterar ou remover a senha, use **Arquivo > Informações > Criptografar com Senha** (veja A tela Arquivo). Uma cópia salva como arquivo .odp mantém a senha também.

## Fontes no arquivo

Uma fonte é o desenho das letras. Nem todo computador tem todas as fontes. Se um computador não tiver uma fonte usada nos seus slides, ele desenha as letras em outra fonte, e os slides podem ficar diferentes.

O JeffDeck pode colocar dentro do arquivo as fontes que os seus slides usam. Assim, os slides ficam iguais em qualquer computador. Isso se chama inserção. O JeffDeck faz isso a cada vez que você salva. Para desativar, clique em **Arquivo > Opções**, abra a guia **Salvar** e desmarque **Inserir fontes no arquivo**.

- O JeffDeck insere apenas as fontes que acompanham o programa, como Fraunces, Lato e Open Sans. Outros computadores não têm essas. Ele nunca insere uma fonte que todo computador tem, como Calibri ou Arial. Deixa de fora uma fonte cuja licença diz que ela não pode ser copiada para um arquivo.
- O JeffDeck insere cada estilo que os slides usam: normal, negrito, itálico e negrito itálico. Se os slides usarem só o estilo normal, só esse entra.
- O JeffDeck insere todas as letras de cada fonte, e não apenas as que você usou. Assim, quem abrir o arquivo ainda pode alterar as palavras.
- O arquivo fica maior. Cada estilo de uma fonte acrescenta cerca de 40 a 700 KB (um KB é mil bytes). O arquivo é compactado ao ser salvo, então cresce cerca de metade desse valor.

Ao abrir um arquivo que tem fontes internas, o JeffDeck as usa enquanto a apresentação estiver aberta. Os slides aparecem como foram criados. O JeffDeck não instala as fontes no seu computador e não as copia para lugar nenhum. Elas pertencem apenas àquela apresentação: não aparecem nas suas listas de fontes, e nenhuma outra apresentação as usa. Elas saem de cena quando você fecha a apresentação. Se o seu computador já tiver uma fonte, o JeffDeck usa a sua própria cópia.

O JeffDeck lê fontes de arquivos .pptx feitos por outros programas de apresentação, mesmo quando o programa as compactou. Ele também lê fontes de arquivos .odp.

Um arquivo pode trazer uma fonte que o JeffDeck não tem. Ao salvar, o JeffDeck coloca essa fonte de volta no arquivo, desde que os slides ainda a usem. Isso acontece com ou sem **Inserir fontes no arquivo** ativado. Se uma fonte do arquivo estiver danificada ou não puder ser lida, a mensagem que você vê ao abrir o arquivo informa isso.

## Tipos de arquivo que o JeffDeck lê

- **.pptx, .potx e .ppsx** são um formato em três versões: apresentação, modelo e apresentação de slides. O JeffDeck lê e grava os três.
- **.ppt, .pps e .pot** são o tipo antigo. O JeffDeck os lê, mas não consegue gravá-los, então **Salvar** pede um novo nome e salva um .pptx (um .ppsx, no caso de um .pps). Ele nunca sobrescreve o original.
- **.odp e .otp** são apresentações em formato OpenDocument e os seus modelos. O JeffDeck lê e grava os dois. Um .odp aberto aqui é salvo como .odp. Um .otp é um modelo, então **Salvar** pede um novo nome e sugere um .odp.

Se parte de um arquivo não puder ser lida, o JeffDeck abre o restante e lista o que deixou de fora. Se o arquivo inteiro não puder ser lido, ele informa o motivo.

## Salvar como um arquivo .odp

Um arquivo .odp abre no LibreOffice Impress com os seus slides, mestres, imagens, anotações e comentários, e com a aparência que o JeffDeck mostra. Ele guarda animações, transições de slides, apresentações personalizadas e as configurações da apresentação de slides. A fonte de cada palavra é salva pelo nome, então um computador que não tenha a fonte mostra outra.

O JeffDeck ainda não lê de volta, a partir de um arquivo .odp, as animações, os comentários, as apresentações personalizadas nem as configurações da apresentação. Eles estão no arquivo, e o LibreOffice os reproduz, mas, quando você abre o arquivo .odp no JeffDeck, ele lista esses itens como deixados de fora. Mantenha o seu arquivo .pptx se quiser continuar editando esses itens aqui.

Um arquivo .odp não tem lugar para algumas coisas. O JeffDeck as lista após a gravação, na mensagem na parte inferior da janela:

- **Gráficos, diagramas e equações** são salvos como imagens. Você pode vê-los, mas não pode editar os números ou as partes deles no arquivo .odp.
- **Seções** não são salvas.
- **Biséis e sombras internas** são deixados de fora, assim como brilhos e reflexos em texto. Uma sombra em texto é mantida. Um reflexo em uma forma, uma linha com gradiente, uma linha de duas ou três espessuras e o tamanho de uma sombra são mantidos de um modo que só o JeffDeck lê. Outros programas mostram a forma sem o reflexo, a linha na sua primeira cor, uma espessura única e a sombra no tamanho do objeto.
- **Efeitos de animação** que começam ao clicar em um objeto e efeitos que alteram uma cor são deixados de fora. A cor para a qual um objeto escurece depois do efeito também é deixada de fora: o efeito permanece, e o objeto mantém a própria cor. Um texto animado por palavra ou por letra é salvo como uma única animação do texto inteiro. Um clique em um vídeo, para pausá-lo ou reproduzi-lo novamente, é mantido.
- **Um vídeo ou som vinculado** a partir de outro arquivo, e não guardado na apresentação, mantém o endereço e a imagem de visualização no arquivo .odp. O LibreOffice o reproduz se conseguir encontrar o arquivo. Vídeos e sons que estão na apresentação são guardados no arquivo .odp e são reproduzidos.

Mantenha também o seu arquivo .pptx, se precisar desses itens de volta.

## O que é mantido, mas não mostrado

Algumas partes de um arquivo são coisas que o JeffDeck ainda não consegue mostrar ou alterar. Ele as mantém com os seus slides e as grava de volta ao salvar, para que não se percam. Entre elas estão as animações, que são reproduzidas na apresentação; vídeos e sons, que ainda não são reproduzidos; e um efeito de transição para o qual o JeffDeck não tem forma, que é reproduzido como esmaecimento. Gráficos e diagramas aparecem como foram salvos. Veja [Bem-vindo ao JeffDeck](start).

## Veja também

- [Recuperar trabalho não salvo](recover)
- [A tela Arquivo](file-screen)
- [Reutilizar slides de outra apresentação](reuse)
- [Impressão, PDF e imagens dos slides](printing)
