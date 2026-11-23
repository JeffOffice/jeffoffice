# Abrir, salvar, imprimir e exportar
<!-- keywords: fontes no arquivo, fontes inseridas, fonte ausente, e-book, ebook, epub, leitor de e-book, capítulos, sumário, página da web, html, salvar como página da web, página da web filtrada, email, eml, indicadores, indicadores de PDF, shift+f5, última edição, onde você parou, modelos integrados, galeria, carta, relatório, resumo, manuscrito, boletim informativo, abrir, salvar, salvar como, docx, doc, dot, odt, opendocument, rtf, arquivos antigos, modelo, dotx, recentes, fixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não salvo, travamento, tela Arquivo, formatos de arquivo, salvar uma cópia, somente leitura, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, ott, reparar, abrir e reparar, danificado, senha, criptografar, proteger com senha -->

Clique em **Arquivo** no canto superior esquerdo para ver a tela Arquivo. Ela tem páginas para Novo, Abrir, Imprimir e Exportar, além dos comandos Salvar e Salvar Como. Pressione **Esc** para voltar ao seu documento.

## Iniciar um novo documento

Pressione **Ctrl+N**, ou clique em **Arquivo > Novo** e depois em **Documento em Branco**. Um novo documento abre em uma janela própria. Se a janela em que você está estiver vazia e nunca tiver sido salva, o novo documento toma o lugar dela.

## Abrir um documento

Pressione **Ctrl+O**, ou clique em **Arquivo > Abrir**. Clique em **Procurar** para escolher um arquivo. O JeffWrite abre estes tipos de arquivo:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo abre como um novo documento feito a partir dele, de modo que o modelo continua como está.
- **.docm** e **.dotm** são um documento e um modelo que podem conter **macros**, pequenos programas que acompanham um arquivo. O JeffWrite nunca os executa, e os mantém como estavam quando você salva como .docm ou .dotm. Se você salvar como .docx ou .odt, as macros são deixadas de fora, e o JeffWrite avisa você.
- **.txt** é texto simples, apenas as palavras. Ao abrir um, o JeffWrite o lê de imediato se ele estiver em ASCII simples ou Unicode. Se não estiver, o JeffWrite pergunta em qual codificação lê-lo e mostra as primeiras linhas, para que você veja se as letras estão corretas. Uma **codificação** é a forma como um arquivo armazena as letras, como UTF-8 ou Windows 1252. Ao salvar como .txt, você escolhe a codificação e o fim de linha, e o arquivo guarda apenas as palavras: sem fontes, imagens ou linhas de tabela.
- **.html** e **.htm** são páginas da web. O JeffWrite lê as palavras, os títulos, o negrito, o itálico, o sublinhado, as cores, as listas e as tabelas. Os marcadores e números das listas entram como marcas digitadas, e as imagens e os links são deixados de fora. Ele nunca busca nada na internet e ignora scripts. Uma página da web nunca é sobrescrita: **Salvar** pede um novo nome e salva um arquivo .docx. Para criar uma página da web, veja **Salvar como página da web** mais abaixo.
- **.doc** e **.dot** são os tipos mais antigos de documento e modelo. O JeffWrite os lê e salva o que você faz com eles como um novo arquivo .docx.
- **.odt**, **.ott** e **.fodt** são arquivos OpenDocument. O JeffWrite lê os três. Ele salva arquivos .odt e salva o que você faz com um arquivo .ott ou .fodt como um novo arquivo .odt. Veja [Abrir e salvar arquivos OpenDocument](open-odt).
- **.rtf** é Rich Text Format. O JeffWrite o lê e o salva. Veja [Abrir arquivos RTF](open-rtf).

A página **Abrir** também lista os documentos que você abriu ou salvou recentemente. Clique em um para abri-lo. Clique com o botão direito em um para fixá-lo no topo da lista, ou para removê-lo da lista.

Se um arquivo tiver partes que o JeffWrite ainda não consegue editar, como o layout de uma tabela ou um cabeçalho, ele as mantém. Veja [Bem-vindo ao JeffWrite](start). Se alguma parte de um arquivo não puder ser lida de jeito nenhum, o JeffWrite informa o que deixou de fora. Nesse caso, ele não salva por cima desse arquivo: Salvar pede um novo nome, para que o original mantenha o que a cópia perdeu.

## Salvar um documento

Pressione **Ctrl+S** para salvar. Na primeira vez, o JeffWrite pede um nome. **Salvar como** (F12) salva uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para salvar um modelo, **Documento com Macros (*.docm)** para manter macros, **Texto Simples (*.txt)** para guardar apenas as palavras, ou **Rich Text Format (*.rtf)** para um arquivo que outros programas consigam ler. Um arquivo que veio de um tipo mais antigo é sempre salvo com um novo nome, para que o arquivo antigo nunca seja sobrescrito.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse arquivo, e você continua trabalhando no documento que tinha. O nome dele e as alterações não salvas continuam como estavam. Isso é útil quando você quer enviar uma versão a alguém e seguir escrevendo.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na página **Geral**, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffWrite primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome dele, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve salvar sem ela. A resposta é Não, a menos que você diga Sim, e o arquivo continua como estava. Assim você pode voltar à última versão salva.

## Salvar como modelo

Um modelo é um documento que você usa como ponto de partida. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo vai para **Meus Modelos**, e o documento que você está escrevendo não é alterado. Em **Salvar Como**, você também pode escolher **Modelo (*.dotx)**, ou **Modelo de Texto OpenDocument (*.ott)**, para salvar um arquivo de modelo onde preferir.

Para iniciar um novo documento a partir do seu modelo, clique em **Arquivo > Novo**. Seus modelos aparecem em **Meus Modelos**. Clique em um deles. Um novo documento abre com o texto e os estilos do modelo. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Meus Modelos**.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito em um nome na lista de Recentes. Cada forma pede um arquivo, ou usa aquele em que você clicou:

- **Abrir como Somente Leitura** abre o arquivo apenas para leitura. Você pode consultar, rolar, pesquisar e copiar, mas não pode alterar nada. A barra de título mostra "Somente Leitura". Se você pressionar **Salvar**, o JeffWrite pede um novo nome, de modo que o arquivo aberto não é alterado.
- **Abrir como Cópia** abre um novo documento com o texto do arquivo. Ele não tem arquivo próprio, então **Salvar** pergunta onde guardá-lo. O arquivo aberto nunca é alterado.
- **Abrir em Nova Janela** abre o arquivo em uma janela própria, mesmo quando a janela em que você está estiver vazia.
- **Abrir e Reparar** serve para um arquivo .docx ou .doc danificado, por exemplo um que foi cortado ao ser copiado. Veja a seguir.

## Abrir e Reparar

Às vezes um arquivo .docx ou .doc fica danificado. Uma cópia parou no meio, ou um disco cometeu erros. Um arquivo .docx é um conjunto de partes menores, como o texto, os estilos e as imagens. **Abrir e Reparar** lê cada parte que consegue. Uma parte interrompida no meio é mantida até o ponto em que parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem sumido, o JeffWrite as refaz.

Um arquivo .doc antigo é um grande invólucro com alguns fluxos de dados dentro. Um fluxo guarda o texto, e outro guarda a tabela que diz onde está cada trecho de texto e como ele aparece. Se o invólucro estiver cortado ou suas listas estiverem danificadas, **Abrir e Reparar** localiza esses fluxos pelo conteúdo e recupera o máximo que puder. Quando as listas que dizem como as partes se encaixam estão danificadas, o programa considera cada parte como seguindo a anterior, e mostra o que encontra ali. Esse texto pode pertencer a uma cópia antiga do documento ou a outra parte do arquivo, por isso a mensagem indica o parágrafo onde ele começa e pede que você o compare com um backup, se tiver um. Se a suposição vier antes do texto, a mensagem diz que o documento inteiro foi remontado; se vier depois do texto, a mensagem diz que a formatação foi remontada. Se as imagens não puderem ser totalmente recuperadas, a mensagem informa isso. Se a tabela for perdida, o texto é mantido como parágrafos simples, sem estilos, listas, tabelas ou imagens. Um arquivo .doc com senha não pode ser reparado.

O documento reparado abre como cópia, com um nome como "Carta reparada". Uma mensagem informa o que foi mantido até uma interrupção e o que foi deixado de fora. O arquivo danificado não é alterado. Salve a cópia reparada com um nome próprio e confira-a antes de confiar nela.

## Recuperar trabalho não salvo

O JeffWrite salva uma cópia de reserva do trabalho não salvo a cada poucos minutos, como um arquivo em uma pasta própria. Se o programa fechar inesperadamente, ele oferece essas cópias na próxima vez que você o iniciar. Clique em **Arquivo > Abrir > Recuperar Trabalho Não Salvo** para vê-las depois. Você pode escolher quais abrir, e pode salvar cada uma onde quiser.

## Começar a partir de um modelo

**Arquivo > Novo** mostra o **Documento em Branco** e um conjunto de modelos que acompanham o JeffWrite, agrupados por tipo: uma carta, um relatório, um resumo, um manuscrito e um boletim informativo. Cada imagem é a primeira página do documento que você obtém. Um **modelo** é um documento já preparado, com seus estilos, margens e textos de exemplo. Clique em um e substitua os textos de exemplo pelos seus. Os modelos que você mesmo salvou aparecem em **Meus Modelos**.

## Continuar de onde você parou

O JeffWrite lembra onde estava o cursor quando você salvou ou fechou um arquivo. Na próxima vez que abrir o arquivo, o cursor estará ali. Pressione **Shift+F5** para ir aos lugares editados por último. Cada pressionamento vai para o anterior: os três últimos lugares, e depois volta ao primeiro.

## Salvar como .odt

Um arquivo .odt não consegue guardar tudo o que um arquivo .docx guarda. Ao salvar como .odt, o JeffWrite primeiro lista o que se perderia e permite salvar um arquivo .docx. Veja [Informações](info).

## Imprimir **Ctrl+P**, ou clique em **Arquivo > Imprimir**. Escolha uma impressora, as cópias e as páginas, veja cada página na visualização e clique em **Imprimir**. Veja [Imprimir](print).

## Criar um PDF

Clique em **Arquivo > Exportar > Exportar para PDF**. O PDF leva o título, o autor, o assunto e as palavras-chave do documento, vindos de **Arquivo > Informações** (com **Incluir propriedades do documento** marcado), a data em que foi feito e o JeffWrite como o programa que o criou; o autor e um título formado pelo nome do arquivo ficam de fora quando as informações pessoais são removidas. Cada um desses itens é mantido inteiro até 4.000 caracteres. Marque **Abrir o PDF após salvar** para que o arquivo seja aberto no programa de PDF do seu computador ao ser salvo. Essa opção fica desativada até você marcá-la. Se o seu computador não conseguir abrir o arquivo, a barra de status avisa (o arquivo é salvo do mesmo jeito). Escolha quais páginas incluir: todas, a página atual ou um intervalo como 1-3, 5 (p2s3 e s3 também funcionam aqui). Depois escolha como o PDF é feito:

- **Otimizar para** tem **Padrão**, que mantém as imagens como estão, e **Tamanho mínimo**, que faz cada imagem ter no máximo 150 pixels por polegada, para que o arquivo fique menor. Use Tamanho mínimo para um PDF que você vai colocar em uma página da web ou enviar por email.
- **Incluir propriedades do documento** coloca no PDF o título, o autor, o assunto e as palavras-chave do documento.
- **Criar indicadores a partir dos títulos** transforma cada título em um **indicador**, uma entrada da lista que o leitor de PDF mostra ao lado das páginas. Clique em um indicador para ir à página dele.
- **Marcas de estrutura do documento para acessibilidade** marca os títulos, parágrafos, listas, tabelas (com suas linhas de cabeçalho) e imagens (com seus textos alternativos), e informa o idioma, para que um leitor de tela leia o PDF na ordem certa.

Depois escolha um nome. O PDF tem a mesma aparência em qualquer computador, porque as fontes ficam dentro dele. O PDF que **Imprimir** cria ao escolher um arquivo PDF também tem os indicadores e as marcas. Quando ele coloca várias páginas em uma folha, cada indicador e cada link leva à folha que contém a página, e as páginas de uma folha são lidas na ordem em que estão dispostas nela: da esquerda para a direita, e de cima para baixo.

## Salvar como página da web

Clique em **Arquivo > Exportar** e escolha um de dois tipos de página da web. Uma **página da web** é um arquivo que você pode abrir em qualquer navegador.

- **Salvar como Página da Web** cria um único arquivo .html que contém suas palavras, a aparência delas e suas imagens. Você pode enviar esse arquivo sozinho. A aparência mantida é a fonte, o tamanho, a cor, o realce, o negrito, o itálico, o sublinhado, o alinhamento e o espaço entre os parágrafos.
- **Salvar como Página da Web Filtrada** cria um arquivo .html pequeno, com apenas a estrutura: títulos, negrito e itálico, listas, tabelas, links e imagens. Ele não tem fontes, tamanhos ou cores, então você pode dar a ele a sua aparência depois.

Os dois mantêm títulos, listas com marcadores e numeradas, tabelas com células mescladas, links e imagens. O texto oculto, as palavras que uma alteração controlada excluiu, comentários, cabeçalhos, rodapés e notas são deixados de fora. Um link que iniciaria um programa ou executaria um script é salvo como texto simples.

## Salvar como e-book

Um **e-book** é um livro que você lê em um leitor de livros digitais, um celular ou um tablet. O arquivo é um **EPUB**. As palavras se ajustam a qualquer tela, e o leitor pode aumentar ou diminuir as letras.

1. Clique em **Arquivo > Exportar > Salvar como E-book (EPUB)**.
2. Confira o **Título** e o **Autor**. Eles começam com o título e o autor nas propriedades do documento (**Arquivo > Informações**). Você pode alterá-los apenas para este livro.
3. Ative **Usar a primeira imagem como capa** se quiser que a primeira imagem do documento seja a capa. Sem essa opção, o livro não tem capa.
4. Clique em **OK** e escolha onde salvar o arquivo.

Como o livro é feito:

- Cada **Título 1** inicia um novo capítulo. As palavras antes do primeiro Título 1 formam um capítulo próprio.
- O **sumário** mostra cada Título 1, Título 2 e Título 3, e o leitor pode ir a cada um deles. Os títulos menores estão no livro, mas não no sumário.
- Imagens e tabelas ficam onde você as colocou. O texto alternativo de uma imagem vai junto com ela, para que um leitor de tela possa dizer o que ela mostra.
- Notas de rodapé e notas de fim são colocadas no fim do capítulo em que estão. Cada nota tem um link de volta ao ponto do texto, e cada número no texto é um link para a sua nota.

Um livro tem limites, para que um arquivo danificado não gere um livro enorme: até 500 capítulos, 4.000 linhas no sumário, 1.000 imagens (nenhuma com mais de 32 MB) e 20.000 notas. O que passar de um limite fica no último capítulo, ou vira texto simples, e suas palavras nunca são descartadas: uma imagem além do limite vira o seu texto alternativo, e uma nota além do limite perde a formatação. Uma imagem que o leitor não consegue mostrar (um metarquivo, um bitmap ou um TIFF) é convertida para PNG. Exportar o mesmo documento salvo novamente gera o mesmo livro, para que o leitor mantenha o seu lugar.

## Proteger um PDF com senha

Na caixa **Exportar para PDF**, clique em **Criptografar com senha…**. Digite a senha duas vezes. Quem abrir o PDF precisará digitá-la. A senha protege de fato o arquivo: as páginas são embaralhadas com criptografia forte (AES-256), e ninguém consegue lê-las sem ela.

Você também pode marcar **Restringir impressão, cópia e edição**, definir uma segunda senha e escolher o que é permitido. Essas restrições são apenas um pedido. A maioria dos leitores de PDF as respeita, mas elas não impedem quem queira contorná-las, então não conte com elas para manter um arquivo privado.

Uma senha esquecida não pode ser recuperada, então guarde-a em um lugar seguro. A senha vale apenas para esse PDF e nunca é salva. A caixa desativa o **PDF/A** enquanto uma senha estiver definida, porque um arquivo PDF/A não pode ter senha.

## Criar um PDF/A

Na caixa **Exportar para PDF**, marque **PDF/A, para guardar por muito tempo**. Um arquivo PDF/A (PDF/A-1b) guarda todas as fontes que usa, nomeia suas cores e segue um padrão criado para guardar arquivos por muitos anos. Ele pode ter marcas para leitores de tela, e não pode ter senha.

## Criar um email

Clique em **Arquivo > Exportar > Criar um E-mail** para criar um arquivo de mensagem (.eml). Nada é enviado. Você escolhe o que vai na mensagem: o documento, o documento como PDF, ou as páginas exibidas como imagens na mensagem (as primeiras 30 páginas). Abra o arquivo no seu programa de email, informe para quem ele é destinado e envie. Para um PDF, você pode clicar em **Criptografar com senha…** na caixa; informe a senha à pessoa por outro meio, não na mensagem.

## Fontes no arquivo

Um arquivo .docx pode trazer as fontes em que o texto está composto, para que o documento apareça como o autor o fez, mesmo em um computador que não tenha essas fontes. O JeffWrite as lê ao abrir o arquivo. Elas são usadas apenas para esse documento, enquanto ele estiver aberto. Nunca são instaladas no seu computador, nenhum outro documento ou programa as vê, e somem ao fechar o documento. Uma fonte que o arquivo traz não conta como fonte ausente.

O JeffWrite não usa uma fonte cuja licença do fabricante não permita incorporá-la, uma fonte danificada, nem nenhuma fonte além das 64 primeiras de um arquivo. Quando isso acontece, a mensagem que você vê ao abrir o arquivo informa, e o texto é exibido em uma fonte semelhante. O arquivo mantém as fontes como estavam quando você o salva.

Para colocar nos arquivos que você salva as fontes usadas no texto, clique em **Arquivo > Opções**, abra a página **Salvar** e ative **Salvar fontes no arquivo**. Essa opção vem desativada. Então, ao salvar um arquivo .docx, o JeffWrite inclui as fontes que vêm com ele e que o texto usa, em cada estilo usado pelo documento. Ele nunca inclui uma fonte cuja licença não permita, e, quando deixa uma de fora, a mensagem após o salvamento informa o nome dela. Isso aumenta o tamanho do arquivo, e no máximo 64 fontes e 256 MB cabem em um único arquivo.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Usar o JeffWrite com o teclado](keyboard)
