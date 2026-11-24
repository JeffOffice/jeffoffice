# Abrir, salvar, imprimir e exportar
<!-- keywords: fontes no arquivo, fontes inseridas, fonte ausente, e-book, ebook, epub, leitor de e-book, capítulos, sumário, página da web, html, salvar como página da web, página da web filtrada, email, eml, indicadores, indicadores de pdf, shift+f5, última edição, onde parei, modelos integrados, galeria, carta, carta de apresentação, carta comercial, currículo, fatura, pauta de reunião, ata de reunião, folheto, relatório, resumo, manuscrito, boletim informativo, abrir, salvar, salvar como, docx, doc, dot, odt, opendocument, rtf, arquivos antigos, modelo, dotx, recentes, fixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não salvo, travamento, tela de arquivo, formatos de arquivo, salvar uma cópia, somente leitura, leitura somente, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, ott, reparar, abrir e reparar, danificado, senha, criptografar, proteger com senha -->

Clique em **Arquivo** no canto superior esquerdo para ver a tela Arquivo. Ela tem páginas para Novo, Abrir, Imprimir e Exportar, além dos comandos Salvar e Salvar Como. Pressione **Esc** para voltar ao seu documento.

## Iniciar um novo documento

Pressione **Ctrl+N**, ou clique em **Arquivo > Novo** e depois em **Documento em Branco**. Um novo documento é aberto em sua própria janela. Se a janela em que você está estiver vazia e nunca tiver sido salva, o novo documento toma o lugar dela.

## Abrir um documento

Pressione **Ctrl+O**, ou clique em **Arquivo > Abrir**. Clique em **Procurar** para escolher um arquivo. O JeffWrite abre estes tipos de arquivo:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo é aberto como um novo documento feito a partir dele, de modo que o modelo permanece como está.
- **.docm** e **.dotm** são um documento e um modelo que podem conter **macros**, pequenos programas que acompanham um arquivo. O JeffWrite nunca os executa; ele os mantém como estavam quando você salva como .docm ou .dotm. Se você salvar como .docx ou .odt, as macros são deixadas de fora, e o JeffWrite avisa você.
- **.txt** é texto simples, apenas as palavras. Ao abrir um, o JeffWrite o lê imediatamente se ele estiver em ASCII simples ou Unicode. Se não estiver, o JeffWrite pergunta em qual codificação ler e mostra as primeiras linhas, para que você possa ver se as letras estão corretas. Uma **codificação** é a forma como um arquivo armazena letras, como UTF-8 ou Windows 1252. Ao salvar como .txt, você escolhe a codificação e o final das linhas, e o arquivo guarda apenas as palavras: sem fontes, imagens ou linhas de tabela.
- **.html** e **.htm** são páginas da web. O JeffWrite lê as palavras, os títulos, o negrito, o itálico, o sublinhado, as cores, as listas e as tabelas. Marcadores e números de listas entram como marcas digitadas, e imagens e links são deixados de fora. Ele nunca busca nada na internet e ignora scripts. Uma página da web nunca é sobrescrita: **Salvar** pede um novo nome e salva um arquivo .docx. Para criar uma página da web, veja **Salvar como página da web** abaixo.
- **.doc** e **.dot** são os tipos mais antigos de documento e modelo. O JeffWrite os lê e salva o que você fizer com eles como um novo arquivo .docx.
- **.odt**, **.ott** e **.fodt** são arquivos OpenDocument. O JeffWrite lê os três. Ele salva arquivos .odt e salva o que você fizer com um arquivo .ott ou .fodt como um novo arquivo .odt. Veja [Abrir e salvar arquivos OpenDocument](open-odt).
- **.rtf** é Rich Text Format. O JeffWrite lê e salva esse formato. Veja [Abrir arquivos RTF](open-rtf).

A página **Abrir** também lista os documentos que você abriu ou salvou recentemente. Clique em um para abri-lo. Clique com o botão direito em um para fixá-lo no topo da lista ou removê-lo da lista.

Se um arquivo tiver partes que o JeffWrite ainda não consegue editar, como o layout de uma tabela ou um cabeçalho, ele as mantém. Veja [Bem-vindo ao JeffWrite](start). Se uma parte do arquivo não puder ser lida de jeito nenhum, o JeffWrite informa o que deixou de fora. Nesse caso, ele não salva por cima desse arquivo: Salvar pede um novo nome, para que o original preserve o que a cópia perdeu.

## Salvar um documento

Pressione **Ctrl+S** para salvar. Na primeira vez, o JeffWrite pede um nome. **Salvar Como** (F12) salva uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para salvar um modelo, **Documento com Macros (*.docm)** para manter macros, **Texto Simples (*.txt)** para guardar somente as palavras, ou **Rich Text Format (*.rtf)** para um arquivo que outros programas possam ler. Um arquivo que veio de um tipo mais antigo é sempre salvo com um novo nome, para que o arquivo antigo nunca seja sobrescrito.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse arquivo, e você continua trabalhando no documento que tinha aberto. O nome dele e as alterações não salvas permanecem como estavam. Isso é útil quando você quer enviar uma versão a alguém e continuar escrevendo.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na página **Geral**, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffWrite primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome dele, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve salvar sem ela. A resposta é Não, a menos que você diga Sim, e o arquivo permanece como estava. Assim, você pode voltar à última versão salva.

## Salvar como modelo

Um modelo é um documento que você usa como ponto de partida. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo vai para **Meus Modelos**, e o documento que você está escrevendo não é alterado. Em **Salvar Como**, você também pode escolher **Modelo (*.dotx)** ou **Modelo de Texto OpenDocument (*.ott)** para salvar um arquivo de modelo onde preferir.

Para iniciar um novo documento a partir do seu modelo, clique em **Arquivo > Novo**. Seus modelos aparecem em **Meus Modelos**. Clique em um deles. Um novo documento é aberto com o texto e os estilos do modelo. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Meus Modelos**.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito em um nome na lista de Recentes. Cada forma pede um arquivo ou usa aquele em que você clicou:

- **Abrir Somente Leitura** abre o arquivo apenas para leitura. Você pode olhar, rolar, pesquisar e copiar, mas não pode alterar nada. A barra de título mostra "Somente Leitura". Se você pressionar **Salvar**, o JeffWrite pede um novo nome, de modo que o arquivo aberto não é alterado.
- **Abrir como Cópia** abre um novo documento com o texto do arquivo. Ele não tem um arquivo próprio, então **Salvar** pergunta onde guardá-lo. O arquivo aberto nunca é alterado.
- **Abrir em Nova Janela** abre o arquivo em uma janela própria, mesmo quando a janela em que você está estiver vazia.
- **Abrir e Reparar** serve para um arquivo .docx ou .doc danificado, por exemplo um que foi cortado ao ser copiado. Veja abaixo.

## Abrir e Reparar

Às vezes um arquivo .docx ou .doc é danificado. Uma cópia parou no meio do caminho, ou um disco cometeu erros. Um arquivo .docx é um conjunto de partes menores, como o texto, os estilos e as imagens. **Abrir e Reparar** lê todas as partes que consegue. Uma parte que foi interrompida no meio é mantida até o ponto em que parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem se perdido, o JeffWrite as recria.

Um arquivo .doc antigo é um grande invólucro com alguns fluxos de dados dentro. Um fluxo guarda o texto, e outro guarda a tabela que diz onde está cada trecho de texto e como ele se apresenta. Se o invólucro for cortado ou se suas listas estiverem danificadas, **Abrir e Reparar** localiza esses fluxos pelo conteúdo e recupera o máximo possível. Quando as listas que dizem como os trechos se encaixam estão danificadas, o programa pega cada trecho para segui-lo ao anterior e mostra o que encontra ali. Esse texto pode pertencer a uma cópia antiga do documento ou a outra parte do arquivo, por isso a mensagem indica o parágrafo onde ele começa e pede que você o compare com um backup, se tiver um. Se o palpite vier antes do texto, a mensagem diz que o documento inteiro foi remontado; se vier depois do texto, a mensagem diz que a formatação foi remontada. Se as imagens não puderem ser totalmente recuperadas, a mensagem informa isso. Se a tabela for perdida, o texto é mantido como parágrafos simples, sem estilos, listas, tabelas ou imagens. Um arquivo .doc que tenha senha não pode ser reparado.

O documento reparado é aberto como uma cópia com um nome como "Carta reparada". Uma mensagem informa o que foi mantido até uma interrupção e o que foi deixado de fora. O arquivo danificado não é alterado. Salve a cópia reparada com um nome próprio e verifique-a antes de confiar nela.

## Recuperar trabalho não salvo

O JeffWrite salva uma cópia de reserva do trabalho não salvo a cada poucos minutos, como um arquivo em uma pasta própria. Se o programa fechar inesperadamente, ele oferece essas cópias na próxima vez que você o iniciar. Clique em **Arquivo > Abrir > Recuperar Trabalho Não Salvo** para vê-las mais tarde. Você pode escolher quais abrir e salvar cada uma onde preferir.

## Começar a partir de um modelo

**Arquivo > Novo** mostra o **Documento em Branco** e um conjunto de modelos que acompanham o JeffWrite, agrupados por tipo: cartas (uma carta pessoal, uma carta de apresentação e uma carta comercial), relatórios (um relatório, um relatório com página de rosto e sumário, e um resumo), um manuscrito, publicações (um boletim informativo e um folheto), currículos (um clássico e outro com coluna lateral) e documentos comerciais (uma fatura, uma pauta de reunião e uma ata de reunião). Aponte para uma imagem para ver para que serve o modelo. Cada imagem é a primeira página do documento que você recebe. Um **modelo** é um documento já configurado, com seus estilos, margens e palavras de exemplo. Clique em um e substitua as palavras de exemplo pelas suas. Os modelos que você mesmo salvou aparecem em **Meus Modelos**.

## Continuar de onde você parou

O JeffWrite lembra onde estava o cursor quando você salvou ou fechou um arquivo. Na próxima vez que abrir o arquivo, o cursor estará ali. Pressione **Shift+F5** para ir aos lugares editados por último. Cada pressionamento vai ao anterior: os últimos três lugares e, depois, volta ao primeiro.

## Salvar como .odt

Um arquivo .odt não pode guardar tudo o que um arquivo .docx guarda. Ao salvar como .odt, o JeffWrite primeiro lista o que se perderia e permite salvar um arquivo .docx. Veja [Informações](info).

## Imprimir

Pressione **Ctrl+P**, ou clique em **Arquivo > Imprimir**. Escolha uma impressora, as cópias e as páginas, veja cada página na visualização e clique em **Imprimir**. Veja [Imprimir](print).

## Criar um PDF

Clique em **Arquivo > Exportar > Exportar para PDF**. O PDF leva o título, o autor, o assunto e as palavras-chave de **Arquivo > Informações** (com **Incluir propriedades do documento** marcado), a data em que foi criado e o JeffWrite como o programa que o criou. O autor e um título formado pelo nome do arquivo ficam de fora quando as informações pessoais são removidas. Cada um desses campos é mantido inteiro, até 4.000 caracteres. Marque **Abrir o PDF após salvar** para que o arquivo seja aberto no programa de PDF do seu computador quando for salvo. Essa opção fica desativada até que você a marque. Se o seu computador não conseguir abrir o arquivo, a barra de status informa isso (o arquivo é salvo do mesmo jeito). Escolha quais páginas incluir: todas, a página atual ou um intervalo como 1-3, 5 (p2s3 e s3 também funcionam aqui). Depois, escolha como o PDF será feito:

- **Otimizar para** tem **Padrão**, que mantém as imagens como estão, e **Tamanho mínimo**, que faz cada imagem não ter mais de 150 pixels por polegada, para que o arquivo fique menor. Use Tamanho mínimo para um PDF que você vai publicar em uma página da web ou enviar por email.
- **Incluir propriedades do documento** coloca no PDF o título, o autor, o assunto e as palavras-chave do documento.
- **Criar indicadores a partir dos títulos** transforma cada título em um **indicador**, uma entrada na lista que um leitor de PDF mostra ao lado das páginas. Clique em um indicador para ir à sua página.
- **Marcas de estrutura do documento para acessibilidade** marca os títulos, os parágrafos, as listas, as tabelas (com suas linhas de cabeçalho) e as imagens (com seu texto alternativo), e indica o idioma, para que um leitor de tela leia o PDF na ordem certa.

Depois, escolha um nome. O PDF tem a mesma aparência em qualquer computador, porque as fontes ficam dentro dele. O PDF que **Imprimir** cria ao escolher um arquivo PDF também tem os indicadores e as marcas. Quando ele coloca várias páginas em uma folha, cada indicador e cada link leva à folha que contém sua página, e as páginas de uma folha são lidas na ordem em que estão dispostas nela: da esquerda para a direita, depois de cima para baixo.

## Salvar como página da web

Clique em **Arquivo > Exportar** e escolha um dos dois tipos de página da web. Uma **página da web** é um arquivo que você pode abrir em qualquer navegador.

- **Salvar como Página da Web** cria um único arquivo .html que contém suas palavras, a aparência delas e suas imagens. Você pode enviar esse arquivo sozinho. A aparência mantida é a fonte, o tamanho, a cor, o realce, o negrito, o itálico, o sublinhado, o alinhamento e o espaço entre os parágrafos.
- **Salvar como Página da Web Filtrada** cria um arquivo .html pequeno, com apenas a estrutura: títulos, negrito e itálico, listas, tabelas, links e imagens. Ele não tem fontes, tamanhos nem cores, para que você possa definir a aparência depois.

Os dois mantêm títulos, listas com marcadores e numeradas, tabelas com células mescladas, links e imagens. Texto oculto, as palavras que uma alteração controlada excluiu, comentários, cabeçalhos, rodapés e notas são deixados de fora. Um link que iniciaria um programa ou executaria um script é salvo como texto simples.

## Salvar como e-book

Um **e-book** é um livro que você lê em um leitor de livros digitais, um celular ou um tablet. O arquivo é um **EPUB**. O texto se ajusta a qualquer tela, e o leitor pode aumentar ou diminuir as letras.

1. Clique em **Arquivo > Exportar > Salvar como E-book (EPUB)**.
2. Confira o **Título** e o **Autor**. Eles começam com o título e o autor nas propriedades do documento (**Arquivo > Informações**). Você pode alterá-los apenas para este livro.
3. Ative **Usar a primeira imagem como capa** se quiser que a primeira imagem do documento seja a capa. Sem essa opção, o livro não tem capa.
4. Clique em **OK** e escolha onde salvar o arquivo.

Como o livro é feito:

- Cada **Título 1** começa um novo capítulo. As palavras antes do primeiro Título 1 formam um capítulo próprio.
- O **sumário** mostra todos os Títulos 1, 2 e 3, e o leitor pode ir a cada um deles. Títulos menores estão no livro, mas não no sumário.
- Imagens e tabelas ficam onde você as colocou. O texto alternativo de uma imagem acompanha a imagem, para que um leitor de tela possa dizer o que ela mostra.
- Notas de rodapé e notas de fim são colocadas no fim do capítulo em que estão. Cada nota tem um link de volta ao ponto do texto, e cada número no texto é um link para sua nota.

Um livro tem limites, para que um arquivo danificado não gere um arquivo enorme: até 500 capítulos, 4.000 linhas no sumário, 1.000 imagens (nenhuma maior que 32 MB) e 20.000 notas. O que passar de um limite fica no último capítulo ou se torna texto simples, e as suas palavras nunca são perdidas: uma imagem além do limite vira o seu texto alternativo, e uma nota além do limite perde a formatação. Uma imagem que o leitor não consegue mostrar (um metarquivo, um bitmap ou um TIFF) é convertida em PNG. Exportar de novo o mesmo documento salvo gera o mesmo livro, para que o leitor mantenha o seu lugar.

## Proteger um PDF com senha

Na caixa **Exportar para PDF**, clique em **Criptografar com senha…**. Digite a senha duas vezes. Quem abrir o PDF precisará digitá-la. A senha protege de fato o arquivo: as páginas são embaralhadas com criptografia forte (AES-256), e ninguém consegue lê-las sem ela.

Você também pode marcar **Restringir impressão, cópia e edição**, informar uma segunda senha e escolher o que é permitido. Essas restrições são apenas um pedido. A maioria dos leitores de PDF as respeita, mas elas não impedem quem quer contorná-las, então não conte com elas para manter um arquivo privado.

Uma senha esquecida não pode ser recuperada, por isso guarde-a em um lugar seguro. A senha vale apenas para esse PDF e nunca é salva. Enquanto houver uma senha definida, a caixa desativa **PDF/A**, pois um arquivo PDF/A não pode ter senha.

## Criar um PDF/A

Na caixa **Exportar para PDF**, marque **PDF/A, para guardar por muito tempo**. Um arquivo PDF/A (PDF/A-1b) contém todas as fontes que usa, identifica as suas cores e segue um padrão criado para guardar arquivos por muitos anos. Ele pode ter marcações para leitores de tela, e não pode ter senha.

## Criar um email

Clique em **Arquivo > Exportar > Criar um Email** para gerar um arquivo de mensagem (.eml). Nada é enviado. Você escolhe o que vai na mensagem: o documento, o documento como PDF ou as páginas exibidas como imagens na mensagem (as primeiras 30 páginas). Abra o arquivo no seu programa de email, informe o destinatário e envie. Para um PDF, você pode clicar em **Criptografar com senha…** na caixa; informe a senha à pessoa por outro meio que não a própria mensagem.

## Fontes no arquivo

Um arquivo .docx pode carregar as fontes em que o seu texto está composto, para que o documento tenha a aparência que o autor lhe deu, mesmo em um computador que não tenha essas fontes. O JeffWrite as lê ao abrir o arquivo. Elas são usadas apenas por esse documento, enquanto ele estiver aberto. Nunca são instaladas no seu computador, nenhum outro documento ou programa as vê, e somem quando você fecha o documento. Uma fonte que o arquivo traz não conta como fonte ausente.

O JeffWrite não usa uma fonte cuja licença do fabricante proíba a inserção, uma fonte danificada, nem qualquer fonte além das 64 primeiras de um arquivo. Quando isso acontece, a mensagem que você vê ao abrir o arquivo informa, e o texto é exibido em uma fonte semelhante. O arquivo mantém suas fontes como estavam quando você o salvar.

Para incluir no arquivo que você salvar as fontes usadas no texto, clique em **Arquivo > Opções**, abra a página **Salvar** e ative **Inserir fontes no arquivo**. Essa opção vem desativada. Então, ao salvar um arquivo .docx, o JeffWrite inclui as fontes que vêm com ele e que o texto usa, em cada estilo usado no documento. Ele nunca inclui uma fonte cuja licença não permita isso, e quando deixa uma de fora, a mensagem após o salvamento informa o nome dela. Isso aumenta o tamanho do arquivo, e no máximo 64 fontes e 256 MB entram em um mesmo arquivo.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Usar o JeffWrite com o teclado](keyboard)
